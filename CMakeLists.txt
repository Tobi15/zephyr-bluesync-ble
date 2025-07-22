if(CONFIG_BLUESYNC_SUPPORT)
  zephyr_library()

  zephyr_library_sources(
    src/bluesync.c
    src/local_time.c
    src/bs_state_machine.c
    src/bluesync_bitfields.c
  )

  zephyr_include_directories(include)

  # Only compile BabbleSim file if explicitly enabled
  zephyr_include_directories($ENV{BSIM_COMPONENTS_PATH}/libUtilv1/src)
  zephyr_library_sources_ifdef(CONFIG_BLUESYNC_TEST_BABBLESIM_SUPPORT src/statistic/bluesync_statistic_bsim.c)
endif()
