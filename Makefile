CC=clang++
CFLAGS=-O3 -ffast-math -I./sdv_tools
LIBS=-lm

VFLAGS=-mepi -mllvm -combiner-store-merging=0 -Rpass=loop-vectorize -Rpass-analysis=loop-vectorize -mcpu=avispado -mllvm -vectorizer-use-vp-strided-load-store -mllvm -enable-mem-access-versioning=0 -mllvm -disable-loop-idiom-memcpy -fno-slp-vectorize

Kmeans.x: Kmeans.cpp
	$(CC) $(CFLAGS) $(VFLAGS) $(SDV_TRACE_INCL) $^ -o $@ $(LIBS) $(SDV_TRACE_C_LINK)