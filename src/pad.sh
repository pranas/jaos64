SIZE1=`du -b stage1 | awk '{print $1 }'`
SIZE2=`du -b stage2 | awk '{print $1 }'`
COUNT=$(( ((1440 * 1024) - $SIZE1 - $SIZE2) / 512 ))
dd if=/dev/zero of=pad bs=512 count=$COUNT
