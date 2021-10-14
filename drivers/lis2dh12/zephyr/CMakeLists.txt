if(CONFIG_LIS2DH12)
    zephyr_include_directories(.)
    zephyr_library()

    zephyr_library_sources(lis2dh12.c)
    zephyr_library_sources(lis2dh12_spi.c)
endif()