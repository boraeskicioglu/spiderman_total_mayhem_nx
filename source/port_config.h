/* Ultimate Spider-Man: Total Mayhem HD settings for android32. */
#ifndef PORT_CONFIG_H
#define PORT_CONFIG_H

#define PORT_TITLE    "Ultimate Spider-Man: Total Mayhem HD"
#define PORT_NAME     "spiderman_total_mayhem_nx"
#define PORT_PACKAGE  "com.gameloft.android.GAND.GloftSMHP.ML"
#define PORT_BANNER   "spiderman_tm_nx: Ultimate Spider-Man: Total Mayhem HD (Gameloft custom/Irrlicht engine, armeabi)"
#define PORT_ABI_DIR  "lib/armeabi/"

/* APK 1.0.8 libspiderman.so maps to ~5.73 MiB; leave generous room. */
#define PORT_SO_REGION_BYTES (16u * 1024 * 1024)

#define PORT_APK_DESC "Ultimate Spider-Man: Total Mayhem HD 1.0.8 (armeabi)"
#define PORT_APK_ROLES                                                                  \
  {.what = "the game",                                                                 \
   .need = (const char *const[]){"lib/armeabi/libspiderman.so",                        \
                                 "lib/armeabi/libStormGLOFT.so", NULL},                 \
   .flags = RT_APK_HIGHEST_VERSION}
#define RT_PACKAGE_MISMATCH_FATAL 1

#define PORT_LAUNCHER_START_NOTE "(Stage 0 unpacks and validates libspiderman.so from your APK)"
#define PORT_LAUNCHER_BYLINE     "Unofficial homebrew wrapper; bring your own APK"

/* Future full port: the game expects /sdcard/gameloft/games/GloftSMHP/. */
#define RT_PATH_SD_SUBDIRS "gameloft"
#define RT_PATH_EXTRA_DIRS "gameloft", "gameloft/games", "gameloft/games/GloftSMHP"

#define RT_PAD_MAX_PLAYERS 1
#define RT_SETUP_UPDATE_PERMILLE 500

#endif

#define RT_GL_CHECK 1
