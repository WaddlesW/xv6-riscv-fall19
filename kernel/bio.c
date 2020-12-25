// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

struct {
  //构建13个哈希组
  struct spinlock lock[13];
  struct buf buf[NBUF];

  // Linked list of all buffers, through prev/next.
  // head.next is most recently used.
  struct buf head[13];
} bcache;

int
bhead(int blockno)
{
  return blockno%13;
}

void
binit(void)
{
  struct buf *b;

  //initlock(&bcache.lock, "bcache");   分别初始化
  for(int i=0;i<13;i++){
    initlock(&bcache.lock[i], "bcache.bucket");
    b=&bcache.head[i];
    b->prev = b;
    b->next = b;
  }

  // Create linked list of buffers
  //bcache.head.prev = &bcache.head;
  //bcache.head.next = &bcache.head;
  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    // 先将全部buffer放到0号bucket,待后续分配
    b->next = bcache.head[0].next;
    b->prev = &bcache.head[0];
    initsleeplock(&b->lock, "buffer");
    bcache.head[0].next->prev = b;
    bcache.head[0].next = b;
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.

//修改bget()和brelse()使得查找和释放缓存中的不同块时，锁之间的冲突更少。
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;
  int h=bhead(blockno);
  acquire(&bcache.lock[h]);

  // Is the block already cached?
  for(b = bcache.head[h].next; b != &bcache.head[h]; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.lock[h]);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached; recycle an unused buffer.
  // 在其他bucket找到空缓存块后插入原bucket，当下一个要探索的bucket重新回到h时说明所有buffer都busy
  int nexth=(h+1)%13;
  while(nexth != h){
    acquire(&bcache.lock[nexth]);
    for(b = bcache.head[nexth].prev; b != &bcache.head[nexth]; b = b->prev){
      if(b->refcnt == 0) {
        b->dev = dev;
        b->blockno = blockno;
        b->valid = 0;
        b->refcnt = 1;
        // 原链表断开插入到对应bucket
        b->next->prev=b->prev;
        b->prev->next=b->next;
        release(&bcache.lock[nexth]);
        b->next=bcache.head[h].next;
        b->prev=&bcache.head[h];
        bcache.head[h].next->prev=b;
        bcache.head[h].next=b;
        release(&bcache.lock[h]);
        acquiresleep(&b->lock);
        return b;
      }
    }
    // 未找到则释放当前bucket锁，并转到下一个bucket
    release(&bcache.lock[nexth]);
    nexth=(nexth+1)%13;
  }
  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b->dev, b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b->dev, b, 1);
}

// Release a locked buffer.
// Move to the head of the MRU list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);
  int h=bhead(b->blockno);
  acquire(&bcache.lock[h]);
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    b->next->prev = b->prev;
    b->prev->next = b->next;
    b->next = bcache.head[h].next;
    b->prev = &bcache.head[h];
    bcache.head[h].next->prev = b;
    bcache.head[h].next = b;
  }
  
  release(&bcache.lock[h]);
}

void
bpin(struct buf *b) {
  int h=bhead(b->blockno);
  acquire(&bcache.lock[h]);
  b->refcnt++;
  release(&bcache.lock[h]);
}

void
bunpin(struct buf *b) {
  int h=bhead(b->blockno);
  acquire(&bcache.lock[h]);
  b->refcnt--;
  release(&bcache.lock[h]);
}


