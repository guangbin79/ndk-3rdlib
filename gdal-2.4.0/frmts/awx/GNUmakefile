
include ../../GDALmake.opt

CPPFLAGS :=	-I$(GDAL_ROOT)/frmts/raw

OBJ	=	awxdataset.o



default:	$(OBJ:.o=.$(OBJ_EXT))

clean:
	rm -f *.o

install-obj:	$(O_OBJ:.o=.$(OBJ_EXT))
