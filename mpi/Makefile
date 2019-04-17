########################################################################
# Makefile for Parallellized Gram-Schmidt Orthonormalization.
#
########################################################################

CC         =  mpicc
CCFLAGS    =  -O3
LIBS       =  -lmpi -lm


all:
	@echo -e "Usage: "
	@echo -e "   make sgs - serial GS" 
	@echo -e "   make pgs_c - parallell GS with contiguous matrices "
	@echo -e "   make pgs_nc - parallell GS with non-contiguous matrices"
	@echo -e "   make pgs_fc - fast parallell GS with contiguous matrices"



sgs:           sgs.c 
	$(CC) $(CCFLAGS) -o sgs sgs.c $(LIBS)
pgs_c:           pgs_c.c 
	$(CC) $(CCFLAGS) -o pgs_c pgs_c.c $(LIBS)
pgs_nc:           pgs_nc.c 
	$(CC) $(CCFLAGS) -o pgs_nc pgs_nc.c $(LIBS)
pgs_fc:           pgs_fc.c 
	$(CC) $(CCFLAGS) -o pgs_fc pgs_fc.c $(LIBS)

clean:
	$(RM) sgs pgs_c pgs_nc pgs_fc

