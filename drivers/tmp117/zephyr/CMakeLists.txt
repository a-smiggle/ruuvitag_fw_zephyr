if(CONFIG_TMP117)
    zephyr_include_directories(.)
    zephyr_library()

    zephyr_library_sources(tmp117.c)
    zephyr_library_sources(tmp117_i2c.c)
endif()