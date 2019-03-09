add_library(sentencecounterfilter SHARED sentencecounterfilter.cc sentencecountersession.cc)
target_link_libraries(sentencecounterfilter maxscale-common mysqlcommon)
set_target_properties(sentencecounterfilter PROPERTIES VERSION "1.0.0" LINK_FLAGS -Wl,-z,defs)
install_module(sentencecounterfilter core)
