set(exe_name
    cws_single_bigram_pretag
)

set(base_name
    cws_single_pretag
)
set(specific_headers
    ${segmentor_base_dir}/${base_name}/${base_name}_model.h
)
set(specific_libs
    ${segmentor_base_dir}/${base_name}/${base_name}_model.cpp
)


add_executable(${exe_name}
               ${exe_name}.cpp
               ${single_input_bigram_modelhandler_headers} # model handler
               ${specific_headers} # model
               ${single_input_model_headers}        # base model
               ${cws_common_headers}                # common header
               ${specific_libs}
               ${single_input_model_libs} 
               ${cws_common_libs}
               )
               
target_link_libraries(${exe_name}
                      cnn
                      ${Boost_LIBRARIES})
