command_SUBDIR = make -C
SUBDIR_4g-lat = 4g-lat
SUBDIR_liblmmax = liblmmax
SUBDIR_libam = libs3decoder/libam
SUBDIR_libcommon = libs3decoder/libcommon
SUBDIR_libdict = libs3decoder/libdict
SUBDIR_liblm = libs3decoder/liblm_Ng
SUBDIR_libsearch = libs3decoder/libsearch
SUBDIR_util = libsphinxbase/util

all:
	$(command_SUBDIR) $(SUBDIR_4g-lat)
	$(command_SUBDIR) $(SUBDIR_liblmmax)
	$(command_SUBDIR) $(SUBDIR_libam)
	$(command_SUBDIR) $(SUBDIR_libcommon)
	$(command_SUBDIR) $(SUBDIR_libdict)
	$(command_SUBDIR) $(SUBDIR_liblm)
	$(command_SUBDIR) $(SUBDIR_libsearch)
	$(command_SUBDIR) $(SUBDIR_util)
	make 4glat
	make dmp2max
	make lm_convert
	make max_read
	make s3astar

4glat_OBJECTS = main_dag.o $(SUBDIR_4g-lat)/*.o $(SUBDIR_liblm)/*.o $(SUBDIR_libdict)/*.o $(SUBDIR_libcommon)/*.o $(SUBDIR_libam)/*.o $(SUBDIR_util)/*.o
dmp2max_OBJECTS = dmp2max.o $(SUBDIR_liblmmax)/*.o $(SUBDIR_liblm)/*.o $(SUBDIR_libdict)/*.o $(SUBDIR_libcommon)/*.o $(SUBDIR_libam)/*.o $(SUBDIR_util)/*.o
lm_convert_OBJECTS = main_lm_convert.o $(SUBDIR_liblm)/*.o $(SUBDIR_libdict)/*.o $(SUBDIR_libcommon)/*.o $(SUBDIR_libam)/*.o $(SUBDIR_util)/*.o
max_read_OBJECTS = lm_max_read.o $(SUBDIR_liblmmax)/*.o $(SUBDIR_liblm)/*.o $(SUBDIR_libdict)/*.o $(SUBDIR_libcommon)/*.o $(SUBDIR_libam)/*.o $(SUBDIR_util)/*.o
main_astar_OBJECTS = main_astar.o $(SUBDIR_liblmmax)/*.o $(SUBDIR_libam)/*.o $(SUBDIR_libcommon)/*.o $(SUBDIR_libdict)/*.o $(SUBDIR_liblm)/*.o $(SUBDIR_libsearch)/*.o $(SUBDIR_util)/*.o

INCLUDES = -I./include/sphinx3 -I./include/sphinxbase -I./include/lmmax -I./include/4g-lat

.c.o:
	gcc -g -O2 -Wall $(INCLUDES) -c -o $@ $<

4glat: $(4glat_OBJECTS)
	gcc -g -O2 -Wall -lm -pthread -o programs/$@ $(4glat_OBJECTS)

dmp2max: $(dmp2max_OBJECTS)
	gcc -g -O2 -Wall -lm -pthread -o programs/$@ $(dmp2max_OBJECTS)

lm_convert: $(lm_convert_OBJECTS)
	gcc -g -O2 -Wall -lm -pthread -o programs/$@ $(lm_convert_OBJECTS)

max_read: $(max_read_OBJECTS)
	gcc -g -O2 -Wall -lm -pthread -o programs/$@ $(max_read_OBJECTS)

s3astar: $(main_astar_OBJECTS)
	gcc -g -O2 -Wall -lm -pthread -o programs/$@ $(main_astar_OBJECTS)

clean:
	rm -f *.o
	rm -f programs/4glat
	rm -f programs/dmp2max
	rm -f programs/lm_convert
	rm -f programs/max_read
	rm -f programs/s3astar
	$(command_SUBDIR) $(SUBDIR_4g-lat) clean
	$(command_SUBDIR) $(SUBDIR_liblmmax) clean
	$(command_SUBDIR) $(SUBDIR_libam) clean
	$(command_SUBDIR) $(SUBDIR_libcommon) clean
	$(command_SUBDIR) $(SUBDIR_libdict) clean
	$(command_SUBDIR) $(SUBDIR_liblm) clean
	$(command_SUBDIR) $(SUBDIR_libsearch) clean
	$(command_SUBDIR) $(SUBDIR_util) clean
