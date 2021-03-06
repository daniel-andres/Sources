CXXTEMPLFLAGS =  
## -fno-implicit-templates

AM_CPPFLAGS = -I${top_srcdir} -I${top_srcdir}/.. -I${top_builddir} -I${top_builddir}/.. -I${srcdir} -I${top_srcdir}/../factory/include -I${top_builddir}/../factory/include ${FACTORY_CFLAGS} ${GMP_CFLAGS} ${NTL_CFLAGS}

lib_LTLIBRARIES = libcoeffs.la libcoeffs_g.la
libcoeffsdir = $(libdir)/singular

libcoeffs_la_CFLAGS   = -O3 -fomit-frame-pointer ${PIPE}
libcoeffs_g_la_CFLAGS = ${PIPE}

libcoeffs_la_CXXFLAGS   = -O3 -fomit-frame-pointer ${PIPE} ${CXXTEMPLFLAGS}
libcoeffs_g_la_CXXFLAGS = -g -Wextra -Wall -pedantic -Wno-long-long ${PIPE} ${CXXTEMPLFLAGS}
# -fdiagnostics-show-option 

libcoeffs_la_CPPFLAGS   = ${AM_CPPFLAGS} -DHAVE_CONFIG_H -DNDEBUG -DOM_NDEBUG
libcoeffs_g_la_CPPFLAGS = ${AM_CPPFLAGS} -DHAVE_CONFIG_H


# noinst_HEADERS= \
#	gnumpc.h gnumpfl.h longrat.h modulop.h ffields.h \
#	rintegers.h rmodulo2m.h rmodulon.h shortfl.h \
#	mpr_complex.h mpr_global.h


SOURCES = \
	numbers.cc rintegers.cc rmodulo2m.cc rmodulon.cc shortfl.cc \
	gnumpc.cc gnumpfl.cc longrat.cc longrat0.cc ffields.cc \
	modulop.cc mpr_complex.cc \
  bigintmat.cc 

libcoeffs_la_SOURCES   = $(SOURCES)
libcoeffs_g_la_SOURCES = $(SOURCES)

libcoeffs_includedir  =$(includedir)/singular/coeffs
libcoeffs_include_HEADERS = coeffs.h numbers.h si_gmp.h gnumpc.h gnumpfl.h longrat.h modulop.h ffields.h rintegers.h rmodulo2m.h rmodulon.h shortfl.h mpr_complex.h mpr_global.h \
  bigintmat.h 


## for testing...
if ENABLE_FACTORY
  USE_FACTORY = -L${top_builddir}/../factory
else
  USE_FACTORY =
endif

AMLDFLAGS = -L${abs_top_builddir}/coeffs -L${abs_top_builddir}/reporter -L${top_builddir}/misc ${USE_FACTORY} -L${top_builddir}/../omalloc -L${abs_top_builddir}/../findexec

TESTS_ENVIRONMENT = SINGULARPATH='${top_srcdir}/../factory:${top_builddir}/../factory'
TESTS_ENVIRONMENT += SINGULAR_ROOT_DIR='$(top_builddir)' 


TESTS = test_s_g test_s_r test_d_g test_d_r
check_PROGRAMS = $(TESTS)

test_s_r_SOURCES = test.cc
test_s_g_SOURCES = test.cc

test_d_r_SOURCES = test.cc
test_d_g_SOURCES = test.cc


test_s_r_LDADD = -lcoeffs -lreporter -lmisc ${FACTORY_LIBS} -lomalloc -lfindexec ${NTL_LIBS} ${GMP_LIBS} 
test_s_g_LDADD = -lcoeffs_g -lreporter_g -lmisc_g ${FACTORY_LIBS} -lomalloc_g -lfindexec_g ${NTL_LIBS} ${GMP_LIBS} 

test_d_r_LDADD = -lcoeffs -lreporter -lmisc ${FACTORY_LIBS} -lomalloc -lfindexec ${NTL_LIBS} ${GMP_LIBS} 
test_d_g_LDADD = -lcoeffs_g -lreporter_g -lmisc_g ${FACTORY_LIBS} -lomalloc_g -lfindexec_g ${NTL_LIBS} ${GMP_LIBS} 


test_s_r_CFLAGS = -O3 -fomit-frame-pointer ${PIPE}
test_s_g_CFLAGS = ${PIPE}

test_d_r_CFLAGS = -O3 -fomit-frame-pointer ${PIPE}
test_d_g_CFLAGS = ${PIPE}



test_s_r_CXXFLAGS = -O3 -fomit-frame-pointer ${PIPE} ${CXXTEMPLFLAGS} 
# -fimplicit-templates
test_s_g_CXXFLAGS = -g -Wextra -Wall -pedantic -Wno-long-long ${PIPE} ${CXXTEMPLFLAGS}
# -fimplicit-templates
# -fdiagnostics-show-option 
test_d_r_CXXFLAGS = -O3 -fomit-frame-pointer ${PIPE} ${CXXTEMPLFLAGS} 
test_d_g_CXXFLAGS = -g -Wextra -Wall -pedantic -Wno-long-long ${PIPE} ${CXXTEMPLFLAGS}



test_s_r_CPPFLAGS = ${AM_CPPFLAGS} -DHAVE_CONFIG_H -DNDEBUG -DOM_NDEBUG
test_s_g_CPPFLAGS = ${AM_CPPFLAGS} -DHAVE_CONFIG_H

test_d_r_CPPFLAGS = ${AM_CPPFLAGS} -DHAVE_CONFIG_H -DNDEBUG -DOM_NDEBUG
test_d_g_CPPFLAGS = ${AM_CPPFLAGS} -DHAVE_CONFIG_H


test_s_r_LDFLAGS = -static ${AMLDFLAGS}
test_s_g_LDFLAGS = -static ${AMLDFLAGS}

test_d_r_LDFLAGS = ${AMLDFLAGS}
test_d_g_LDFLAGS = ${AMLDFLAGS}
