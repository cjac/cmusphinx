# -*- Makefile -*-

CXX = g++
CC  = gcc

LIBPATH = -L/usr/lib

INCPATH =  -I${INCDIR} -I/usr/include/g++ -I/usr/tools/include/g++ -I/usr/tools/include -I../src -I/usr/local/include/g++ -I/usr/include

# math library
LDLIBS = -lm  

# optimizations for this architecture
# ARCHOPTIM = -mcpu=pentiumpro

# for optimizing
OPTIM = -O3 -ffast-math -mcpu=pentiumpro

# for seeing warnings and debugging
#OPTIM = -g -DDEBUG=3
# for profiling
#OPTIM = -g -pg -O3

# this is required for old SunOS
#OPTIONS = -D__USE_FIXED_PROTOTYPES__ -DFLOAT=float -fexceptions -frtti -m486
#OPTIONS = -mcpu=pentiumpro

#WARNS = -Wall -fguiding-decls
#WARNS = -Wall
WARNS  = -w

CXXFLAGS = ${OPTIM} ${OPTIONS} ${WARNS} ${LIBPATH} ${INCPATH}
CCFLAGS  = ${OPTIM} ${OPTIONS} ${WARNS} ${LIBPATH} ${INCPATH}
CFLAGS   = $(CCFLAGS)
LD = ${CXX} ${CXXFLAGS}

# 2 lines for sunOS
CCC = ${CXX}

BINDIR  = ../bin
INCDIR  = ../include
SRCDIR  = ../src
TESTDIR = ../test

TARGETS = ${BINDIR}/Consensus ${BINDIR}/Consensus_print


all: includes ${TARGETS}

clean:
	-${RM} *.o ../lib/*.a *~

sterile: clean
	-${RM} *.rpo
	-${RM} ${TARGETS}

distclean: sterile
	-${RM} makefile

remake: clean depend
	${MAKE}

# figure out the dependencies

depend:
	-$(RM) makefile
	grep -v '^makefile' Make.depend > makefile
	echo "" >> makefile
	echo "# Automatically generated dependencies" >> makefile
	echo "" >> makefile
	-${CXX} ${INCPATH} -M *.cc  >> makefile


Consensus:      ${INCDIR}/common.h  ${INCDIR}/GetOpt.h ${INCDIR}/Lattice.h ${INCDIR}/Prob.h  ${INCDIR}/LineSplitter.h ${INCDIR}/Prons.h ${INCDIR}/zio.h ${INCDIR}/Cluster.h ${INCDIR}/Clustering.h ${INCDIR}/Link.h ${INCDIR}/Node.h ${INCDIR}/common_lattice.h ${INCDIR}/Similarities.h ${SRCDIR}/common.cc ${SRCDIR}/GetOpt.cc ${SRCDIR}/Lattice.cc ${SRCDIR}/LineSplitter.cc ${SRCDIR}/Prons.cc ${SRCDIR}/zio.cc ${SRCDIR}/Cluster.cc ${SRCDIR}/Clustering.cc ${SRCDIR}/Similarities.cc common.o GetOpt.o Lattice.o LineSplitter.o Prons.o zio.o Cluster.o Clustering.o Similarities.o
	$(CCC) $(CCFLAGS) -o ${BINDIR}/Consensus Consensus.cc common.o GetOpt.o Lattice.o LineSplitter.o Prons.o zio.o Cluster.o Clustering.o Similarities.o $(LDLIBS)


Consensus_print:	${INCDIR}/common.h  ${INCDIR}/GetOpt.h ${INCDIR}/Lattice.h ${INCDIR}/Prob.h  ${INCDIR}/LineSplitter.h ${INCDIR}/Prons.h ${INCDIR}/zio.h ${INCDIR}/Cluster.h ${INCDIR}/Clustering.h ${INCDIR}/Link.h ${INCDIR}/Node.h ${INCDIR}/common_lattice.h ${INCDIR}/Similarities.h ${SRCDIR}/common.cc ${SRCDIR}/GetOpt.cc ${SRCDIR}/Lattice.cc ${SRCDIR}/LineSplitter.cc ${SRCDIR}/Prons.cc ${SRCDIR}/zio.cc ${SRCDIR}/Cluster.cc ${SRCDIR}/Clustering_print.cc ${SRCDIR}/Similarities.cc common.o GetOpt.o Lattice.o LineSplitter.o Prons.o zio.o Cluster.o Clustering_print.o Similarities.o
	$(CCC) $(CCFLAGS) -o ${BINDIR}/Consensus_print Consensus_print.cc common.o GetOpt.o Lattice.o LineSplitter.o Prons.o zio.o Cluster.o Clustering_print.o Similarities.o $(LDLIBS)			

# Automatically generated dependencies

Cluster.o: Cluster.cc /usr/include/stdlib.h /usr/include/features.h \
 /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/stddef.h \
 /usr/include/sys/types.h /usr/include/bits/types.h \
 /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/sigset.h /usr/include/sys/sysmacros.h \
 /usr/include/alloca.h /usr/include/g++/iomanip.h \
 /usr/include/g++/iostream.h /usr/include/g++/streambuf.h \
 /usr/include/libio.h /usr/include/_G_config.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/stdarg.h \
 /usr/include/stdio.h /usr/include/bits/stdio_lim.h \
 /usr/include/assert.h /usr/include/g++/algo.h \
 /usr/include/g++/algobase.h /usr/include/g++/pair.h \
 /usr/include/g++/stl_config.h /usr/include/g++/stl_relops.h \
 /usr/include/g++/stl_pair.h /usr/include/g++/iterator.h \
 /usr/include/g++/function.h /usr/include/g++/stl_function.h \
 /usr/include/g++/stl_iterator.h /usr/include/g++/type_traits.h \
 /usr/include/g++/stl_construct.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/new.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/new \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/exception \
 /usr/include/g++/stl_raw_storage_iter.h \
 /usr/include/g++/stl_algobase.h /usr/include/string.h \
 /usr/include/limits.h /usr/include/bits/posix1_lim.h \
 /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
 /usr/include/bits/posix2_lim.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/limits.h \
 /usr/include/g++/stl_uninitialized.h /usr/include/g++/tempbuf.h \
 /usr/include/g++/stl_tempbuf.h /usr/include/g++/stl_algo.h \
 /usr/include/g++/stl_heap.h /usr/include/g++/stl_numeric.h \
 /usr/include/g++/list.h /usr/include/g++/alloc.h \
 /usr/include/g++/stl_alloc.h /usr/include/pthread.h \
 /usr/include/sched.h /usr/include/bits/sched.h \
 /usr/include/bits/time.h /usr/include/signal.h \
 /usr/include/bits/pthreadtypes.h /usr/include/bits/sigthread.h \
 /usr/include/g++/stl_list.h ../include/common_lattice.h \
 /usr/include/g++/map.h /usr/include/g++/tree.h \
 /usr/include/g++/stl_tree.h /usr/include/g++/stl_map.h \
 /usr/include/g++/hash_map.h /usr/include/g++/stl_hashtable.h \
 /usr/include/g++/stl_vector.h /usr/include/g++/stl_hash_fun.h \
 /usr/include/g++/stl_hash_map.h /usr/include/g++/vector.h \
 /usr/include/g++/slist.h /usr/include/g++/stl_slist.h \
 /usr/include/g++/string /usr/include/g++/std/bastring.h \
 /usr/include/g++/cstddef /usr/include/g++/std/straits.h \
 /usr/include/g++/cctype /usr/include/ctype.h /usr/include/g++/cstring \
 /usr/include/g++/iterator /usr/include/g++/std/bastring.cc \
 ../include/Link.h ../include/Prob.h /usr/include/math.h \
 /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h \
 /usr/include/bits/mathcalls.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/float.h ../include/Prons.h \
 ../include/common.h /usr/include/g++/fstream.h \
 /usr/include/g++/hashtable.h ../include/Node.h ../include/Cluster.h \
 ../include/Lattice.h ../include/zio.h /usr/include/g++/stdiostream.h \
 /usr/include/unistd.h /usr/include/bits/posix_opt.h \
 /usr/include/bits/confname.h /usr/include/getopt.h \
 ../include/LatticeInfo.h ../include/Similarities.h \
 ../include/LineSplitter.h
Clustering.o: Clustering.cc /usr/include/stdlib.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/include/gnu/stubs.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/stddef.h \
 /usr/include/sys/types.h /usr/include/bits/types.h \
 /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/sigset.h /usr/include/sys/sysmacros.h \
 /usr/include/alloca.h /usr/include/g++/iomanip.h \
 /usr/include/g++/iostream.h /usr/include/g++/streambuf.h \
 /usr/include/libio.h /usr/include/_G_config.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/stdarg.h \
 /usr/include/stdio.h /usr/include/bits/stdio_lim.h \
 /usr/include/assert.h /usr/include/g++/algo.h \
 /usr/include/g++/algobase.h /usr/include/g++/pair.h \
 /usr/include/g++/stl_config.h /usr/include/g++/stl_relops.h \
 /usr/include/g++/stl_pair.h /usr/include/g++/iterator.h \
 /usr/include/g++/function.h /usr/include/g++/stl_function.h \
 /usr/include/g++/stl_iterator.h /usr/include/g++/type_traits.h \
 /usr/include/g++/stl_construct.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/new.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/new \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/exception \
 /usr/include/g++/stl_raw_storage_iter.h \
 /usr/include/g++/stl_algobase.h /usr/include/string.h \
 /usr/include/limits.h /usr/include/bits/posix1_lim.h \
 /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
 /usr/include/bits/posix2_lim.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/limits.h \
 /usr/include/g++/stl_uninitialized.h /usr/include/g++/tempbuf.h \
 /usr/include/g++/stl_tempbuf.h /usr/include/g++/stl_algo.h \
 /usr/include/g++/stl_heap.h /usr/include/g++/stl_numeric.h \
 /usr/include/g++/list.h /usr/include/g++/alloc.h \
 /usr/include/g++/stl_alloc.h /usr/include/pthread.h \
 /usr/include/sched.h /usr/include/bits/sched.h \
 /usr/include/bits/time.h /usr/include/signal.h \
 /usr/include/bits/pthreadtypes.h /usr/include/bits/sigthread.h \
 /usr/include/g++/stl_list.h /usr/include/sys/stat.h \
 /usr/include/bits/stat.h /usr/include/fcntl.h \
 /usr/include/bits/fcntl.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/confname.h \
 /usr/include/getopt.h ../include/common_lattice.h \
 /usr/include/g++/map.h /usr/include/g++/tree.h \
 /usr/include/g++/stl_tree.h /usr/include/g++/stl_map.h \
 /usr/include/g++/hash_map.h /usr/include/g++/stl_hashtable.h \
 /usr/include/g++/stl_vector.h /usr/include/g++/stl_hash_fun.h \
 /usr/include/g++/stl_hash_map.h /usr/include/g++/vector.h \
 /usr/include/g++/slist.h /usr/include/g++/stl_slist.h \
 /usr/include/g++/string /usr/include/g++/std/bastring.h \
 /usr/include/g++/cstddef /usr/include/g++/std/straits.h \
 /usr/include/g++/cctype /usr/include/ctype.h /usr/include/g++/cstring \
 /usr/include/g++/iterator /usr/include/g++/std/bastring.cc \
 ../include/Link.h ../include/Prob.h /usr/include/math.h \
 /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h \
 /usr/include/bits/mathcalls.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/float.h ../include/Prons.h \
 ../include/common.h /usr/include/g++/fstream.h \
 /usr/include/g++/hashtable.h ../include/Node.h \
 ../include/Clustering.h ../include/Cluster.h ../include/Lattice.h \
 ../include/zio.h /usr/include/g++/stdiostream.h \
 ../include/LatticeInfo.h ../include/Similarities.h \
 ../include/LineSplitter.h
Clustering_print.o: Clustering_print.cc /usr/include/stdlib.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/include/gnu/stubs.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/stddef.h \
 /usr/include/sys/types.h /usr/include/bits/types.h \
 /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/sigset.h /usr/include/sys/sysmacros.h \
 /usr/include/alloca.h /usr/include/g++/iomanip.h \
 /usr/include/g++/iostream.h /usr/include/g++/streambuf.h \
 /usr/include/libio.h /usr/include/_G_config.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/stdarg.h \
 /usr/include/stdio.h /usr/include/bits/stdio_lim.h \
 /usr/include/assert.h /usr/include/g++/algo.h \
 /usr/include/g++/algobase.h /usr/include/g++/pair.h \
 /usr/include/g++/stl_config.h /usr/include/g++/stl_relops.h \
 /usr/include/g++/stl_pair.h /usr/include/g++/iterator.h \
 /usr/include/g++/function.h /usr/include/g++/stl_function.h \
 /usr/include/g++/stl_iterator.h /usr/include/g++/type_traits.h \
 /usr/include/g++/stl_construct.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/new.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/new \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/exception \
 /usr/include/g++/stl_raw_storage_iter.h \
 /usr/include/g++/stl_algobase.h /usr/include/string.h \
 /usr/include/limits.h /usr/include/bits/posix1_lim.h \
 /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
 /usr/include/bits/posix2_lim.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/limits.h \
 /usr/include/g++/stl_uninitialized.h /usr/include/g++/tempbuf.h \
 /usr/include/g++/stl_tempbuf.h /usr/include/g++/stl_algo.h \
 /usr/include/g++/stl_heap.h /usr/include/g++/stl_numeric.h \
 /usr/include/g++/list.h /usr/include/g++/alloc.h \
 /usr/include/g++/stl_alloc.h /usr/include/pthread.h \
 /usr/include/sched.h /usr/include/bits/sched.h \
 /usr/include/bits/time.h /usr/include/signal.h \
 /usr/include/bits/pthreadtypes.h /usr/include/bits/sigthread.h \
 /usr/include/g++/stl_list.h /usr/include/sys/stat.h \
 /usr/include/bits/stat.h /usr/include/fcntl.h \
 /usr/include/bits/fcntl.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/confname.h \
 /usr/include/getopt.h ../include/common_lattice.h \
 /usr/include/g++/map.h /usr/include/g++/tree.h \
 /usr/include/g++/stl_tree.h /usr/include/g++/stl_map.h \
 /usr/include/g++/hash_map.h /usr/include/g++/stl_hashtable.h \
 /usr/include/g++/stl_vector.h /usr/include/g++/stl_hash_fun.h \
 /usr/include/g++/stl_hash_map.h /usr/include/g++/vector.h \
 /usr/include/g++/slist.h /usr/include/g++/stl_slist.h \
 /usr/include/g++/string /usr/include/g++/std/bastring.h \
 /usr/include/g++/cstddef /usr/include/g++/std/straits.h \
 /usr/include/g++/cctype /usr/include/ctype.h /usr/include/g++/cstring \
 /usr/include/g++/iterator /usr/include/g++/std/bastring.cc \
 ../include/Link.h ../include/Prob.h /usr/include/math.h \
 /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h \
 /usr/include/bits/mathcalls.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/float.h ../include/Prons.h \
 ../include/common.h /usr/include/g++/fstream.h \
 /usr/include/g++/hashtable.h ../include/Node.h \
 ../include/Clustering.h ../include/Cluster.h ../include/Lattice.h \
 ../include/zio.h /usr/include/g++/stdiostream.h \
 ../include/LatticeInfo.h ../include/Similarities.h \
 ../include/LineSplitter.h
Consensus.o: Consensus.cc /usr/include/stdlib.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/include/gnu/stubs.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/stddef.h \
 /usr/include/sys/types.h /usr/include/bits/types.h \
 /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/sigset.h /usr/include/sys/sysmacros.h \
 /usr/include/alloca.h /usr/include/limits.h \
 /usr/include/bits/posix1_lim.h /usr/include/bits/local_lim.h \
 /usr/include/linux/limits.h /usr/include/bits/posix2_lim.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/limits.h \
 /usr/include/assert.h /usr/include/g++/iostream.h \
 /usr/include/g++/streambuf.h /usr/include/libio.h \
 /usr/include/_G_config.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/stdarg.h \
 /usr/include/g++/fstream.h /usr/include/g++/iomanip.h \
 /usr/include/ctype.h /usr/include/g++/string \
 /usr/include/g++/std/bastring.h /usr/include/g++/cstddef \
 /usr/include/g++/std/straits.h /usr/include/g++/cctype \
 /usr/include/g++/cstring /usr/include/string.h \
 /usr/include/g++/alloc.h /usr/include/g++/stl_config.h \
 /usr/include/g++/stl_alloc.h /usr/include/pthread.h \
 /usr/include/sched.h /usr/include/bits/sched.h \
 /usr/include/bits/time.h /usr/include/signal.h \
 /usr/include/bits/pthreadtypes.h /usr/include/bits/sigthread.h \
 /usr/include/g++/iterator /usr/include/g++/stl_relops.h \
 /usr/include/g++/stl_iterator.h /usr/include/g++/std/bastring.cc \
 /usr/include/sys/stat.h /usr/include/bits/stat.h \
 ../include/common_lattice.h /usr/include/g++/map.h \
 /usr/include/g++/tree.h /usr/include/g++/stl_tree.h \
 /usr/include/g++/stl_algobase.h /usr/include/g++/stl_pair.h \
 /usr/include/g++/type_traits.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/new.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/new \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/exception \
 /usr/include/g++/stl_construct.h /usr/include/g++/stl_function.h \
 /usr/include/g++/algobase.h /usr/include/g++/pair.h \
 /usr/include/g++/iterator.h /usr/include/g++/function.h \
 /usr/include/g++/stl_raw_storage_iter.h \
 /usr/include/g++/stl_uninitialized.h /usr/include/g++/stl_map.h \
 /usr/include/g++/hash_map.h /usr/include/g++/stl_hashtable.h \
 /usr/include/g++/stl_tempbuf.h /usr/include/g++/stl_algo.h \
 /usr/include/g++/stl_heap.h /usr/include/g++/stl_vector.h \
 /usr/include/g++/stl_hash_fun.h /usr/include/g++/stl_hash_map.h \
 /usr/include/g++/vector.h /usr/include/g++/slist.h \
 /usr/include/g++/stl_slist.h /usr/include/g++/list.h \
 /usr/include/g++/stl_list.h ../include/zio.h /usr/include/stdio.h \
 /usr/include/bits/stdio_lim.h /usr/include/g++/stdiostream.h \
 /usr/include/unistd.h /usr/include/bits/posix_opt.h \
 /usr/include/bits/confname.h /usr/include/getopt.h ../include/Prob.h \
 /usr/include/math.h /usr/include/bits/huge_val.h \
 /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/float.h \
 ../include/GetOpt.h ../include/LatticeInfo.h ../include/Link.h \
 ../include/Prons.h ../include/common.h /usr/include/g++/hashtable.h \
 /usr/include/g++/algo.h /usr/include/g++/tempbuf.h \
 /usr/include/g++/stl_numeric.h ../include/Node.h ../include/Lattice.h \
 ../include/Cluster.h ../include/Similarities.h \
 ../include/LineSplitter.h ../include/Clustering.h
Consensus_print.o: Consensus_print.cc /usr/include/sys/types.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/include/gnu/stubs.h /usr/include/bits/types.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/stddef.h \
 /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/sigset.h /usr/include/sys/sysmacros.h \
 /usr/include/sys/stat.h /usr/include/bits/stat.h \
 /usr/include/stdlib.h /usr/include/alloca.h /usr/include/limits.h \
 /usr/include/bits/posix1_lim.h /usr/include/bits/local_lim.h \
 /usr/include/linux/limits.h /usr/include/bits/posix2_lim.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/limits.h \
 /usr/include/assert.h /usr/include/g++/iostream.h \
 /usr/include/g++/streambuf.h /usr/include/libio.h \
 /usr/include/_G_config.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/stdarg.h \
 /usr/include/g++/fstream.h /usr/include/g++/iomanip.h \
 /usr/include/ctype.h /usr/include/g++/string \
 /usr/include/g++/std/bastring.h /usr/include/g++/cstddef \
 /usr/include/g++/std/straits.h /usr/include/g++/cctype \
 /usr/include/g++/cstring /usr/include/string.h \
 /usr/include/g++/alloc.h /usr/include/g++/stl_config.h \
 /usr/include/g++/stl_alloc.h /usr/include/pthread.h \
 /usr/include/sched.h /usr/include/bits/sched.h \
 /usr/include/bits/time.h /usr/include/signal.h \
 /usr/include/bits/pthreadtypes.h /usr/include/bits/sigthread.h \
 /usr/include/g++/iterator /usr/include/g++/stl_relops.h \
 /usr/include/g++/stl_iterator.h /usr/include/g++/std/bastring.cc \
 ../include/common_lattice.h /usr/include/g++/map.h \
 /usr/include/g++/tree.h /usr/include/g++/stl_tree.h \
 /usr/include/g++/stl_algobase.h /usr/include/g++/stl_pair.h \
 /usr/include/g++/type_traits.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/new.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/new \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/exception \
 /usr/include/g++/stl_construct.h /usr/include/g++/stl_function.h \
 /usr/include/g++/algobase.h /usr/include/g++/pair.h \
 /usr/include/g++/iterator.h /usr/include/g++/function.h \
 /usr/include/g++/stl_raw_storage_iter.h \
 /usr/include/g++/stl_uninitialized.h /usr/include/g++/stl_map.h \
 /usr/include/g++/hash_map.h /usr/include/g++/stl_hashtable.h \
 /usr/include/g++/stl_tempbuf.h /usr/include/g++/stl_algo.h \
 /usr/include/g++/stl_heap.h /usr/include/g++/stl_vector.h \
 /usr/include/g++/stl_hash_fun.h /usr/include/g++/stl_hash_map.h \
 /usr/include/g++/vector.h /usr/include/g++/slist.h \
 /usr/include/g++/stl_slist.h /usr/include/g++/list.h \
 /usr/include/g++/stl_list.h ../include/zio.h /usr/include/stdio.h \
 /usr/include/bits/stdio_lim.h /usr/include/g++/stdiostream.h \
 /usr/include/unistd.h /usr/include/bits/posix_opt.h \
 /usr/include/bits/confname.h /usr/include/getopt.h ../include/Prob.h \
 /usr/include/math.h /usr/include/bits/huge_val.h \
 /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/float.h \
 ../include/GetOpt.h ../include/LatticeInfo.h ../include/Link.h \
 ../include/Prons.h ../include/common.h /usr/include/g++/hashtable.h \
 /usr/include/g++/algo.h /usr/include/g++/tempbuf.h \
 /usr/include/g++/stl_numeric.h ../include/Node.h ../include/Lattice.h \
 ../include/Cluster.h ../include/Similarities.h \
 ../include/LineSplitter.h ../include/Clustering.h
GetOpt.o: GetOpt.cc /usr/include/stdlib.h /usr/include/features.h \
 /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/stddef.h \
 /usr/include/sys/types.h /usr/include/bits/types.h \
 /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/sigset.h /usr/include/sys/sysmacros.h \
 /usr/include/alloca.h /usr/include/string.h ../include/GetOpt.h \
 /usr/include/stdio.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/stdarg.h \
 /usr/include/libio.h /usr/include/_G_config.h \
 /usr/include/bits/stdio_lim.h
Lattice.o: Lattice.cc /usr/include/stdlib.h /usr/include/features.h \
 /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/stddef.h \
 /usr/include/sys/types.h /usr/include/bits/types.h \
 /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/sigset.h /usr/include/sys/sysmacros.h \
 /usr/include/alloca.h /usr/include/g++/iomanip.h \
 /usr/include/g++/iostream.h /usr/include/g++/streambuf.h \
 /usr/include/libio.h /usr/include/_G_config.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/stdarg.h \
 /usr/include/stdio.h /usr/include/bits/stdio_lim.h \
 /usr/include/assert.h /usr/include/g++/stdiostream.h \
 /usr/include/g++/algo.h /usr/include/g++/algobase.h \
 /usr/include/g++/pair.h /usr/include/g++/stl_config.h \
 /usr/include/g++/stl_relops.h /usr/include/g++/stl_pair.h \
 /usr/include/g++/iterator.h /usr/include/g++/function.h \
 /usr/include/g++/stl_function.h /usr/include/g++/stl_iterator.h \
 /usr/include/g++/type_traits.h /usr/include/g++/stl_construct.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/new.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/new \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/exception \
 /usr/include/g++/stl_raw_storage_iter.h \
 /usr/include/g++/stl_algobase.h /usr/include/string.h \
 /usr/include/limits.h /usr/include/bits/posix1_lim.h \
 /usr/include/bits/local_lim.h /usr/include/linux/limits.h \
 /usr/include/bits/posix2_lim.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/limits.h \
 /usr/include/g++/stl_uninitialized.h /usr/include/g++/tempbuf.h \
 /usr/include/g++/stl_tempbuf.h /usr/include/g++/stl_algo.h \
 /usr/include/g++/stl_heap.h /usr/include/g++/stl_numeric.h \
 /usr/include/g++/vector.h /usr/include/g++/alloc.h \
 /usr/include/g++/stl_alloc.h /usr/include/pthread.h \
 /usr/include/sched.h /usr/include/bits/sched.h \
 /usr/include/bits/time.h /usr/include/signal.h \
 /usr/include/bits/pthreadtypes.h /usr/include/bits/sigthread.h \
 /usr/include/g++/stl_vector.h /usr/include/math.h \
 /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h \
 /usr/include/bits/mathcalls.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/float.h ../include/Link.h \
 ../include/Prob.h ../include/Prons.h ../include/common.h \
 /usr/include/g++/fstream.h /usr/include/g++/string \
 /usr/include/g++/std/bastring.h /usr/include/g++/cstddef \
 /usr/include/g++/std/straits.h /usr/include/g++/cctype \
 /usr/include/ctype.h /usr/include/g++/cstring \
 /usr/include/g++/iterator /usr/include/g++/std/bastring.cc \
 /usr/include/g++/hashtable.h /usr/include/g++/stl_hashtable.h \
 /usr/include/g++/stl_hash_fun.h ../include/common_lattice.h \
 /usr/include/g++/map.h /usr/include/g++/tree.h \
 /usr/include/g++/stl_tree.h /usr/include/g++/stl_map.h \
 /usr/include/g++/hash_map.h /usr/include/g++/stl_hash_map.h \
 /usr/include/g++/slist.h /usr/include/g++/stl_slist.h \
 /usr/include/g++/list.h /usr/include/g++/stl_list.h ../include/Node.h \
 ../include/Lattice.h ../include/zio.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/confname.h \
 /usr/include/getopt.h ../include/LatticeInfo.h \
 ../include/LineSplitter.h
LineSplitter.o: LineSplitter.cc ../include/LineSplitter.h \
 /usr/include/stdlib.h /usr/include/features.h \
 /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/stddef.h \
 /usr/include/sys/types.h /usr/include/bits/types.h \
 /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/sigset.h /usr/include/sys/sysmacros.h \
 /usr/include/alloca.h /usr/include/g++/string \
 /usr/include/g++/std/bastring.h /usr/include/g++/cstddef \
 /usr/include/g++/std/straits.h /usr/include/g++/cctype \
 /usr/include/ctype.h /usr/include/g++/cstring /usr/include/string.h \
 /usr/include/g++/alloc.h /usr/include/g++/stl_config.h \
 /usr/include/_G_config.h /usr/include/g++/stl_alloc.h \
 /usr/include/g++/iostream.h /usr/include/g++/streambuf.h \
 /usr/include/libio.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/stdarg.h \
 /usr/include/assert.h /usr/include/pthread.h /usr/include/sched.h \
 /usr/include/bits/sched.h /usr/include/bits/time.h \
 /usr/include/signal.h /usr/include/bits/pthreadtypes.h \
 /usr/include/bits/sigthread.h /usr/include/g++/iterator \
 /usr/include/g++/stl_relops.h /usr/include/g++/stl_iterator.h \
 /usr/include/g++/std/bastring.cc /usr/include/g++/vector.h \
 /usr/include/g++/algobase.h /usr/include/g++/pair.h \
 /usr/include/g++/stl_pair.h /usr/include/g++/iterator.h \
 /usr/include/g++/function.h /usr/include/g++/stl_function.h \
 /usr/include/g++/type_traits.h /usr/include/g++/stl_construct.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/new.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/new \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/exception \
 /usr/include/g++/stl_raw_storage_iter.h \
 /usr/include/g++/stl_algobase.h /usr/include/limits.h \
 /usr/include/bits/posix1_lim.h /usr/include/bits/local_lim.h \
 /usr/include/linux/limits.h /usr/include/bits/posix2_lim.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/limits.h \
 /usr/include/g++/stl_uninitialized.h /usr/include/g++/stl_vector.h
Prons.o: Prons.cc /usr/include/stdlib.h /usr/include/features.h \
 /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/stddef.h \
 /usr/include/sys/types.h /usr/include/bits/types.h \
 /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/sigset.h /usr/include/sys/sysmacros.h \
 /usr/include/alloca.h /usr/include/g++/fstream.h \
 /usr/include/g++/iostream.h /usr/include/g++/streambuf.h \
 /usr/include/libio.h /usr/include/_G_config.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/stdarg.h \
 /usr/include/g++/string /usr/include/g++/std/bastring.h \
 /usr/include/g++/cstddef /usr/include/g++/std/straits.h \
 /usr/include/g++/cctype /usr/include/ctype.h /usr/include/g++/cstring \
 /usr/include/string.h /usr/include/g++/alloc.h \
 /usr/include/g++/stl_config.h /usr/include/g++/stl_alloc.h \
 /usr/include/assert.h /usr/include/pthread.h /usr/include/sched.h \
 /usr/include/bits/sched.h /usr/include/bits/time.h \
 /usr/include/signal.h /usr/include/bits/pthreadtypes.h \
 /usr/include/bits/sigthread.h /usr/include/g++/iterator \
 /usr/include/g++/stl_relops.h /usr/include/g++/stl_iterator.h \
 /usr/include/g++/std/bastring.cc /usr/include/stdio.h \
 /usr/include/bits/stdio_lim.h ../include/Prons.h ../include/common.h \
 /usr/include/g++/iomanip.h /usr/include/limits.h \
 /usr/include/bits/posix1_lim.h /usr/include/bits/local_lim.h \
 /usr/include/linux/limits.h /usr/include/bits/posix2_lim.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/limits.h \
 /usr/include/g++/hashtable.h /usr/include/g++/stl_hashtable.h \
 /usr/include/g++/stl_algobase.h /usr/include/g++/stl_pair.h \
 /usr/include/g++/type_traits.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/new.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/new \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/exception \
 /usr/include/g++/stl_construct.h /usr/include/g++/stl_tempbuf.h \
 /usr/include/g++/stl_algo.h /usr/include/g++/stl_heap.h \
 /usr/include/g++/stl_uninitialized.h /usr/include/g++/stl_function.h \
 /usr/include/g++/stl_vector.h /usr/include/g++/stl_hash_fun.h \
 /usr/include/g++/algo.h /usr/include/g++/algobase.h \
 /usr/include/g++/pair.h /usr/include/g++/iterator.h \
 /usr/include/g++/function.h /usr/include/g++/stl_raw_storage_iter.h \
 /usr/include/g++/tempbuf.h /usr/include/g++/stl_numeric.h \
 /usr/include/g++/vector.h /usr/include/math.h \
 /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h \
 /usr/include/bits/mathcalls.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/float.h \
 ../include/common_lattice.h /usr/include/g++/map.h \
 /usr/include/g++/tree.h /usr/include/g++/stl_tree.h \
 /usr/include/g++/stl_map.h /usr/include/g++/hash_map.h \
 /usr/include/g++/stl_hash_map.h /usr/include/g++/slist.h \
 /usr/include/g++/stl_slist.h /usr/include/g++/list.h \
 /usr/include/g++/stl_list.h ../include/LineSplitter.h \
 ../include/zio.h /usr/include/g++/stdiostream.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/confname.h \
 /usr/include/getopt.h
Similarities.o: Similarities.cc ../include/Similarities.h \
 /usr/include/g++/iostream.h /usr/include/g++/streambuf.h \
 /usr/include/libio.h /usr/include/_G_config.h \
 /usr/include/bits/types.h /usr/include/features.h \
 /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/stddef.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/stdarg.h \
 /usr/include/g++/fstream.h /usr/include/g++/iomanip.h \
 /usr/include/stdio.h /usr/include/bits/stdio_lim.h \
 /usr/include/g++/string /usr/include/g++/std/bastring.h \
 /usr/include/g++/cstddef /usr/include/g++/std/straits.h \
 /usr/include/g++/cctype /usr/include/ctype.h /usr/include/endian.h \
 /usr/include/bits/endian.h /usr/include/g++/cstring \
 /usr/include/string.h /usr/include/g++/alloc.h \
 /usr/include/g++/stl_config.h /usr/include/g++/stl_alloc.h \
 /usr/include/stdlib.h /usr/include/sys/types.h /usr/include/time.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/sigset.h /usr/include/sys/sysmacros.h \
 /usr/include/alloca.h /usr/include/assert.h /usr/include/pthread.h \
 /usr/include/sched.h /usr/include/bits/sched.h \
 /usr/include/bits/time.h /usr/include/signal.h \
 /usr/include/bits/pthreadtypes.h /usr/include/bits/sigthread.h \
 /usr/include/g++/iterator /usr/include/g++/stl_relops.h \
 /usr/include/g++/stl_iterator.h /usr/include/g++/std/bastring.cc \
 /usr/include/math.h /usr/include/bits/huge_val.h \
 /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/float.h \
 ../include/LineSplitter.h /usr/include/g++/vector.h \
 /usr/include/g++/algobase.h /usr/include/g++/pair.h \
 /usr/include/g++/stl_pair.h /usr/include/g++/iterator.h \
 /usr/include/g++/function.h /usr/include/g++/stl_function.h \
 /usr/include/g++/type_traits.h /usr/include/g++/stl_construct.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/new.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/new \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/exception \
 /usr/include/g++/stl_raw_storage_iter.h \
 /usr/include/g++/stl_algobase.h /usr/include/limits.h \
 /usr/include/bits/posix1_lim.h /usr/include/bits/local_lim.h \
 /usr/include/linux/limits.h /usr/include/bits/posix2_lim.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/limits.h \
 /usr/include/g++/stl_uninitialized.h /usr/include/g++/stl_vector.h
common.o: common.cc ../include/common.h /usr/include/assert.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/include/gnu/stubs.h /usr/include/g++/iostream.h \
 /usr/include/g++/streambuf.h /usr/include/libio.h \
 /usr/include/_G_config.h /usr/include/bits/types.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/stddef.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/stdarg.h \
 /usr/include/g++/iomanip.h /usr/include/g++/fstream.h \
 /usr/include/stdlib.h /usr/include/sys/types.h /usr/include/time.h \
 /usr/include/endian.h /usr/include/bits/endian.h \
 /usr/include/sys/select.h /usr/include/bits/select.h \
 /usr/include/bits/sigset.h /usr/include/sys/sysmacros.h \
 /usr/include/alloca.h /usr/include/limits.h \
 /usr/include/bits/posix1_lim.h /usr/include/bits/local_lim.h \
 /usr/include/linux/limits.h /usr/include/bits/posix2_lim.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/limits.h \
 /usr/include/g++/string /usr/include/g++/std/bastring.h \
 /usr/include/g++/cstddef /usr/include/g++/std/straits.h \
 /usr/include/g++/cctype /usr/include/ctype.h /usr/include/g++/cstring \
 /usr/include/string.h /usr/include/g++/alloc.h \
 /usr/include/g++/stl_config.h /usr/include/g++/stl_alloc.h \
 /usr/include/pthread.h /usr/include/sched.h /usr/include/bits/sched.h \
 /usr/include/bits/time.h /usr/include/signal.h \
 /usr/include/bits/pthreadtypes.h /usr/include/bits/sigthread.h \
 /usr/include/g++/iterator /usr/include/g++/stl_relops.h \
 /usr/include/g++/stl_iterator.h /usr/include/g++/std/bastring.cc \
 /usr/include/g++/hashtable.h /usr/include/g++/stl_hashtable.h \
 /usr/include/g++/stl_algobase.h /usr/include/g++/stl_pair.h \
 /usr/include/g++/type_traits.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/new.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/new \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/exception \
 /usr/include/g++/stl_construct.h /usr/include/g++/stl_tempbuf.h \
 /usr/include/g++/stl_algo.h /usr/include/g++/stl_heap.h \
 /usr/include/g++/stl_uninitialized.h /usr/include/g++/stl_function.h \
 /usr/include/g++/stl_vector.h /usr/include/g++/stl_hash_fun.h \
 /usr/include/g++/algo.h /usr/include/g++/algobase.h \
 /usr/include/g++/pair.h /usr/include/g++/iterator.h \
 /usr/include/g++/function.h /usr/include/g++/stl_raw_storage_iter.h \
 /usr/include/g++/tempbuf.h /usr/include/g++/stl_numeric.h \
 /usr/include/g++/vector.h /usr/include/math.h \
 /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h \
 /usr/include/bits/mathcalls.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/float.h \
 /usr/include/g++/cstdio /usr/include/stdio.h \
 /usr/include/bits/stdio_lim.h
zio.o: zio.cc /usr/include/stdio.h /usr/include/features.h \
 /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/stddef.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/stdarg.h \
 /usr/include/bits/types.h /usr/include/libio.h \
 /usr/include/_G_config.h /usr/include/bits/stdio_lim.h \
 /usr/include/string.h /usr/include/sys/param.h /usr/include/limits.h \
 /usr/include/bits/posix1_lim.h /usr/include/bits/local_lim.h \
 /usr/include/linux/limits.h /usr/include/bits/posix2_lim.h \
 /usr/lib/gcc-lib/i386-linux/2.95.2/include/limits.h \
 /usr/include/linux/param.h /usr/include/asm/param.h \
 /usr/include/sys/types.h /usr/include/time.h /usr/include/endian.h \
 /usr/include/bits/endian.h /usr/include/sys/select.h \
 /usr/include/bits/select.h /usr/include/bits/sigset.h \
 /usr/include/sys/sysmacros.h /usr/include/fcntl.h \
 /usr/include/bits/fcntl.h /usr/include/sys/stat.h \
 /usr/include/bits/stat.h /usr/include/errno.h \
 /usr/include/bits/errno.h /usr/include/linux/errno.h \
 /usr/include/asm/errno.h ../include/zio.h \
 /usr/include/g++/stdiostream.h /usr/include/g++/iostream.h \
 /usr/include/g++/streambuf.h /usr/include/unistd.h \
 /usr/include/bits/posix_opt.h /usr/include/bits/confname.h \
 /usr/include/getopt.h
