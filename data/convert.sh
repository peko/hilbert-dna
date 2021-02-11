#!/bin/bash
cd ./GCA_015852585.1
for f in chrX.fna chr2L.fna chr2R.fna chr3L.fna chr3R.fna chr4.fna
do
    cat $f | grep -Ev '^>' | tr -d '\n' > ../$f
done
