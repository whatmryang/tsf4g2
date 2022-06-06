#  @file $RCSfile: Makefile,v $
#  general description of this module
#  $Id: Makefile,v 1.5 2008-11-26 08:19:04 jackyai Exp $
#  @author $Author: jackyai $
#  @date $Date: 2008-11-26 08:19:04 $
#  @version 1.0
#  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
#  @note Platform: Linux

include  tsf4g.mk

CLEANTARGET = clean
TESTTARGET = test
UNITTESTTARGET = unittest

MODS = lib_src
MODSCLEAN =$(patsubst %, %.$(CLEANTARGET), $(MODS))
MODSTEST =$(patsubst %, %.$(TESTTARGET), $(MODS))

all: scons $(MODS)

.PHONY:  scons all dep clean test $(MODS) $(MODSCLEAN) $(MODSTEST) $(MODSUNITTEST)


scons:
	scons

$(MODS):
	@echo "Begin to make tsf4g module: '$@' ......"
	cd $@ && $(MAKE);
	@echo "Finish to  make tsf4g module: '$@'"

$(MODSCLEAN):
	@echo "Begin to clean tsf4g module: '$(patsubst %.$(CLEANTARGET),%, $@)' ......"
	cd $(patsubst %.$(CLEANTARGET),%, $@) && $(MAKE) $(CLEANTARGET);
	@echo "Finish to clean tsf4g module: '$(patsubst %.$(CLEANTARGET),%, $@)'"

$(CLEANTARGET): $(MODSCLEAN)


$(MODSTEST):
	@echo "Begin to test tsf4g module: '$(patsubst %.$(TESTTARGET),%, $@)' ......"
	cd $(patsubst %.$(TESTTARGET),%, $@) && $(MAKE) $(TESTTARGET);
	@echo "Finish to test tsf4g module: '$(patsubst %.$(TESTTARGET),%, $@)'"
$(TESTTARGET): $(MODSTEST)
	cd lib/examples/tlog && make test
$(UNITTESTTARGET):
	@echo "Begin to do unit-test for tsf4g module: '$(patsubst %.$(UNITTESTTARGET),%, $@)' ......"
	cd unittests && $(MAKE) $(UNITTESTTARGET);
	@echo "Finish to clean tsf4g module: $@ "

$(UNITTESTTARGET): $(MODSUNITTEST)


install:
	@echo "tsf4g  install  now..."
	mkdir -p $(prefix);
	mkdir -p $(libdir);
	mkdir -p $(bindir);
	mkdir -p $(includedir);
	mkdir -p ${prefix}/services;
	mkdir -p ${prefix}/tools;
	@echo "tsf4g  install  copy include files.."
	cp -r $(TSF4G_INC)/*  $(includedir);
	@echo "tsf4g  install  copy lib files.."
	cp -r $(TSF4G_LIB)/*  $(libdir);
	@echo "tsf4g  install  copy bin files.."
	cp -r $(TSF4G_TOOLS)/*  ${prefix}/tools;
	@echo "tsf4g services"
	cp -r ${TSF4G_SERVICES}/* ${prefix}/services
	@echo "install tormsvr"
	mkdir -p ${prefix}/services/torm_service;
	cp apps/orm/tormsvr/tormsvr ${prefix}/services/torm_service
	cp -r distribution_script/orm_dist/*  ${prefix}/services/torm_service
	@echo "install tconnd"
	mkdir -p ${prefix}/services/tconnd_service;
	cp apps/tconnd/tconnd ${prefix}/services/tconnd_service



