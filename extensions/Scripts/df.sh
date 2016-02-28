#!/bin/sh

# diskspace - summarize available disk space and present in a logical
#    and readable fashion

tempfile="/tmp/available.$$"

#trap "rm -f $tempfile" EXIT

cat << 'EOF' > $tempfile
    { avail += $4 
      total += $2}
END { mb = avail / 1024
      gb = mb / 1024
      tb = gb / 1024
      totmb = total / 1024
      totgb = totmb / 1024
      tottb = totgb / 1024
      if (tottb >= 1 && tb >= 1) printf " Disk space (available/total) = %.2fTB/%.2fTB ", tb, tottb
      else if (tottb >= 1 && tb < 1) printf " Disk space (available/total) = %.2fGB/%.2fTB ", gb, tottb
      else printf " Disk space (available/total) = %.2fGB/%.2fGB " , gb, totgb
    }
EOF

df -k | grep -v rootfs | grep -v udev | grep -v tmpfs | grep -v cgroup_root | awk -f $tempfile

exit 0
