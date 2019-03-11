add_library(operationcounterfilter SHARED operationcounterfilter.cc operationcountersession.cc)
target_link_libraries(operationcounterfilter maxscale-common mysqlcommon)
set_target_properties(operationcounterfilter PROPERTIES VERSION "1.0.0" LINK_FLAGS -Wl,-z,defs)
install_module(operationcounterfilter core)
