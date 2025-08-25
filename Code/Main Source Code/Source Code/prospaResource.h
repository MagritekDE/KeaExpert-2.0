#ifndef PROSPARESOURCE_H
#define PROSPARESOURCE_H

// Common IDs

#define ID_STATIC -1
#define ID_APPLY	 1
#define ID_CANCEL	 2


// Control menu

#define ID_SHOW_CLI              100
#define ID_SHOW_1D               101
#define ID_SHOW_2D               102
#define ID_SHOW_3D               103
#define ID_SHOW_EDITOR           104
#define ID_SET_FOLDER            105
#define ID_SAVE_WINDOW_POSITIONS 106
#define ID_SAVE_DIRECTORIES      107
#define ID_SHOW_PROSPA           108
#define ID_RUN_MACRO             109
#define ID_PREFERENCES           110
#define ID_SAVE_LAYOUT           111
#define ID_ABORT_MACRO           112

// File menu

#define ID_LOAD_PLOT		    200
#define ID_REMOVE_DATA      201
#define ID_CLOSE_PLOT	    202
#define ID_SAVE_PLOT		    203
#define ID_SAVE_PLOT_AS	    204
#define ID_LOAD_DATA	   	 205
#define ID_SAVE_DATA_AS		 206
#define ID_FILE_IMPORT	    207
#define ID_FILE_EXPORT	    208
#define ID_1D_FILE_IMPORT	 209
#define ID_1D_FILE_EXPORT	 210
#define ID_FILE_SAVE_WMF	 211
#define ID_PRINT_1D_PLOT    212
#define ID_APP_EXIT			 213
#define ID_CLOSE_DATA       214
#define ID_CLOSE_PLOTS_KMP  215
#define ID_CLOSE_PLOTS_RMP  216
#define ID_2D_FILE_IMPORT   217
#define ID_2D_FILE_EXPORT   218
#define ID_3D_FILE_IMPORT   219
#define ID_3D_FILE_EXPORT   220
#define ID_GOTO_NEXT_WINDOW 221  
#define ID_GOTO_LAST_WINDOW 222 
#define ID_SHOWHIDE_GUI     223

// View menu

#define ID_ZOOM_REGION				300
#define ID_DISPLAY_LAST_REGION	301
#define ID_DISPLAY_ALL				302
#define ID_TOOLBAR_STATUS			303
#define ID_CENTRE_POINT          304

#define ID_DATA_SELECT			   305 // Don't change order of next 4
#define ID_REGION_SELECT         306
#define ID_ROW_STATUS            307
#define ID_COLUMN_STATUS         308
#define ID_MOVE_PLOT             309

#define ID_2D_DISPLAY_PARAMETERS 310
#define ID_COLOR_BAR             311
#define ID_MAKE_SQUARE           312
#define ID_2D_SURFACE_PLOT       313
#define ID_3D_SURFACE_PLOT       314

#define ID_SHOW_REAL             315
#define ID_SHOW_IMAGINARY        316
#define ID_SHOW_COMPLEX          317

#define ID_1_PLOT                318
#define ID_2_BY_1_PLOT           319
#define ID_1_BY_2_PLOT           320
#define ID_2_BY_2_PLOT           321
#define ID_MULTI_PLOT            322
#define ID_DISPLAY_BORDERS       323
#define ID_HOLD_DISPLAY          324
#define ID_REMOVE_BLANK_PLOTS    325
#define ID_I_BY_J_PLOT           326
#define ID_PLOT1D_AUTOSCALE      327

#define ID_CENTRE_TRACE          328
#define ID_GOTO_NEXT_SUBPLOT     329
#define ID_GOTO_LAST_SUBPLOT     330
#define ID_AUTOSCALE_Y           331

// Axes menu

#define ID_TICKS_N_LABELS	400
#define ID_GRID            401
#define ID_PLOT_RANGE		402
#define ID_WINDOW_COLORS	403
#define ID_LINEAR_X			404
#define ID_LOG_X				405
#define ID_AXES_FONT			406
#define ID_LABEL_FONT		407
#define ID_TITLE_FONT      408
#define ID_TITLE_TXT       409
#define ID_XLABEL_TXT      410
#define ID_YLABEL_TXT      411
#define ID_AUTOTICK		   412
#define ID_HORIZ_Y_LABEL   413
#define ID_VERT_Y_LABEL    414

// Plot menu

#define ID_DEFAULT_TRACE_AND_SYMBOLS	  500
#define ID_CURRENT_TRACE_AND_SYMBOLS	  501
#define ID_EXTRACT_1D_DATA	              502
#define ID_EXTRACT_2D_DATA	              503
#define ID_KEEP_THIS_REGION              504
#define ID_REMOVE_THIS_REGION            505
#define ID_CLEAR_THIS_REGION             506

// Text menu

#define ID_EDITOR       600
#define ID_CLI          601

// Help menu

#define ID_ABOUT	               700
#define ID_GENERAL_HELP          701
#define ID_HELP_VIEWER           702
#define ID_COMMAND_HELP          703



// CLI and Edit menus

#define ID_LOAD_MACRO             800
#define ID_SAVE_MACRO             801
#define ID_SAVE_MACRO_AS          802
#define ID_RUN_TEXT               803
#define ID_HIDE_WINDOW            804
#define ID_GOTO_NEXT_WIN          805
#define ID_CLOSE_MACRO            806
#define ID_CLOSE_ALL_MACROS       807
#define ID_PRINT_MACRO            808
#define ID_SAVE_AND_RUN_MACRO     809

// Edit menu

#define ID_COPY                  900
#define ID_CUT                   901
#define ID_PASTE                 902
#define ID_SELECT_ALL            903
#define ID_UNDO                  904
#define ID_CLEAR_CLI             905
#define ID_FIND_REPLACE          906
#define ID_REDO                  907
#define ID_GOTO_PROCEDURE        908
#define ID_GOTO_LAST_PROCEDURE   909
#define ID_GOTO_NEXT_PROCEDURE   910
#define ID_INDENT_TEXT           911
#define ID_UNINDENT_TEXT         912
#define ID_BLOCK_COMMENT         913
#define ID_BLOCK_UNCOMMENT       914
#define ID_1_EDIT                915
#define ID_2_BY_1_EDIT           916
#define ID_1_BY_2_EDIT           917
#define ID_2_BY_2_EDIT           918
#define ID_GOTO_LINE             919
#define ID_SHOW_ERROR            920
#define ID_SHOW_EDIT_PATH        921
#define ID_EDIT_FINDDOWN         922
#define ID_EDIT_FINDUP           923
#define ID_FINDREPLACE_SEARCHUP  924
#define ID_FINDREPLACE_WRAP      925
#define ID_FINDREPLACE_CASE      926
#define ID_PROCEDURE_MENU        927
#define ID_PROCEDURES_DUMMYITEM1 928
#define ID_PROCEDURES_DUMMYITEM2 929
#define ID_INC_EDIT_FONTSIZE     930
#define ID_DEC_EDIT_FONTSIZE     931
#define ID_REPLACE_TABS          932
#define ID_GOTO_NEXT_SUBEDITOR   933
#define ID_GOTO_LAST_SUBEDITOR   934
#define ID_3_BY_1_EDIT           935
#define ID_I_BY_J_EDIT           936
#define ID_SYNTAX_COLORING       937

// Toolbar ids

#define IDTB_1D_BMP  	1000
#define IDTB_2D_BMP  	1001
#define IDTB_3D_BMP  	1002

#define ID_1D_TOOLBAR	1003
#define ID_2D_TOOLBAR	1004
#define ID_AXES_CORNER	1005 // Don't change the order of the next 3
#define ID_AXES_BOX		1006
#define ID_AXES_CROSS	1007


// Keyboard

#define ID_SHIFT_LEFT  		1100
#define ID_SHIFT_RIGHT		1101
#define ID_SHIFT_UP			1102
#define ID_SHIFT_DOWN		1103

#define ID_ENLARGE_HORIZ  	1104
#define ID_ENLARGE_VERT		1105
#define ID_REDUCE_HORIZ		1106
#define ID_REDUCE_VERT		1107
#define ID_REDUCE_BOTH		1108
#define ID_ENLARGE_BOTH		1109

// Axis menu

#define ID_XTICK_SPACING	   1200
#define ID_YTICK_SPACING	   1201
#define ID_X_TICKS_PER_LABEL	1202
#define ID_Y_TICKS_PER_LABEL	1203
#define ID_TICK_SIZE		   	1204
#define ID_LABEL_SIZE	   	1205
#define ID_X_LABEL_SPACNING   1206
#define ID_Y_LABEL_SPACNING	1207
#define ID_ROTATE_LEFT        1208
#define ID_ROTATE_RIGHT       1209

// Mapping and grid dialog


#define ID_LINEAR_Y		1301
#define ID_LOG_Y			1302
#define ID_XGRID			1303
#define ID_YGRID			1304
#define ID_XFINEGRID		1305
#define ID_YFINEGRID		1306

// Range Dialog

#define ID_MINX			1401
#define ID_MAXX			1402
#define ID_MINY			1403
#define ID_MAXY			1404
#define ID_AUTORANGE    1405

// Colour dialog

#define ID_AXES_COLOR		1501
#define ID_BK_COLOR			1502
#define ID_BORDER_COLOR		1503
#define ID_GRID_COLOR		1504
#define ID_FINE_GRID_COLOR	1505
#define ID_BLACK_N_WHITE	1506
#define ID_DEFAULTCOLORS	1507
#define ID_REAL_COLOR		1508
#define ID_IMAG_COLOR		1509
#define ID_SYMBOL_COLOR		1510

// Trace dialog (note, don't change the order  of this section)

#define TD_LINES					1601
#define TD_STAIRS					1602
#define TD_DOTS					1603
#define TD_NO_TRACE				1604
#define TD_DIAMOND    			1605
#define TD_TRIANGLE	  	 		1606
#define TD_INV_TRIANGLE			1607
#define TD_SQUARE					1608
#define TD_CIRCLE					1609
#define TD_OPEN_DIAMOND    	1610
#define TD_OPEN_TRIANGLE	   1611
#define TD_OPEN_INV_TRIANGLE	1612
#define TD_OPEN_SQUARE			1613
#define TD_OPEN_CIRCLE			1614
#define TD_PLUS         		1615
#define TD_CROSS         		1616
#define TD_NO_SYMBOL				1617

#define ID_SYMBOL_SIZE_CTRL   1618
#define ID_SYMBOL_SIZE_TEXT   1619
#define ID_TRACE_WIDTH_CTRL   1620
#define ID_TRACE_WIDTH_TEXT   1621
#define ID_DEFTRACE_APPLY     1622
#define ID_REAL_STYLE         1623
#define ID_IMAG_STYLE         1624
#define ID_CURTRACE_HELP      1625
#define ID_CURTRACE_CLOSE     1626
#define ID_DEFTRACE_CANCEL    1627
#define ID_ERROR_BAR_COLOR    1628
#define ID_SHOW_ERROR_BARS    1629
#define ID_FIXED_ERRBAR_SIZE  1630
#define ID_FLEXI_ERRBAR_SIZE  1631
#define ID_ERRBAR_SIZE        1632
#define ID_APPLY_ERRBAR       1633
#define ID_DEFTRACE_HELP      1634

// Import dialog

#define ID_ASCII_FILE	      1700
#define ID_BINARY_FILE		   1701
#define ID_32_FLOAT			   1702
#define ID_32_INTEGER		   1703
#define ID_16_INTEGER		   1704
#define ID_XY_DATA			   1705
#define ID_BIG_ENDIAN     	   1706
#define ID_IMPORT_FILENAME	   1707
#define ID_IMPORT_FILE		   1708
#define ID_REAL_DATA          1709
#define ID_COMPLEX_DATA       1710
#define ID_X_LBL              1711
#define ID_Y_LBL              1712
#define ID_X_NAME             1713
#define ID_Y_NAME             1714
#define ID_REALY_NAME         1715
#define ID_IMAGY_NAME         1716
#define ID_REALY_LBL          1717
#define ID_IMAGY_LBL          1718
#define ID_SAVE_TO_VARIABLES  1719
#define ID_DISPLAY_DATA       1720
#define ID_TYPE_STR           1721
#define ID_SIZE_STR           1722
#define ID_X_SIZE             1723
#define ID_X_SIZE_LBL         1724
#define ID_Y_SIZE             1725
#define ID_Y_SIZE_LBL         1726
#define ID_Z_SIZE             1727
#define ID_Z_SIZE_LBL         1728
#define ID_MATRIX_LBL         1729
#define ID_MATRIX_NAME        1730
#define ID_FILE_HEADER        1731
#define ID_ROW_HEADER         1732
#define ID_X_COMPLEX_DATA     1733

// Export dialog (also uses above)

#define ID_SPACE_DELIMIT	1800
#define ID_TAB_DELIMIT		1801
#define ID_COMMA_DELIMIT	1802
#define ID_EXPORT_FILENAME	1803
#define ID_EXPORT_FILE		1804
#define ID_SELECT_FILE     1805

// Print dialog

#define ID_PRINT_COLOR		   1900
#define ID_PRINT_BW			   1901
#define ID_PRINT_WIDTH        1902
#define ID_PRINT_HEIGHT       1903
#define ID_FIT_TO_PAGE        1904
#define ID_LONG_TICK_LENGTH   1905
#define ID_SHORT_TICK_LENGTH  1906
#define ID_SYMBOL_SIZE        1907

// EMF dialog

#define ID_EMF_WIDTH       2000
#define ID_EMF_HEIGHT      2001
#define ID_EMF_SF          2002
#define ID_MATCH_WINDOW    2003

// Text dialog

#define ID_TITLE_TEXT             2100
#define ID_XLABEL_TEXT            2101
#define ID_Y_LEFT_LABEL_TEXT      2102
#define ID_Y_RIGHT_LABEL_TEXT     2103
#define ID_SET_XYLABEL            2104
#define ID_SET_TITLE              2105
#define ID_ALL                    2106


// 2D Colormap menu

#define ID_COLORSCALE_BASE   2200 // Don't put any thing between here and 2300
                                  // These are reserved for colormaps


// 2D View menu

#define ID_1_2D_PLOT                2300
#define ID_2_BY_1_2D_PLOT           2301
#define ID_1_BY_2_2D_PLOT           2302
#define ID_2_BY_2_2D_PLOT           2303
#define ID_I_BY_J_2D_PLOT           2304
#define ID_DISPLAY_ONE_REGION       2305
#define ID_RESTORE_REGIONS          2306
#define ID_PLOT2D_AUTOSCALE         2307

// File menu

#define ID_CLOSE_2D_PLOT 				2400
#define ID_CLOSE_2D_PLOTS_KMP 		2401
#define ID_CLOSE_2D_PLOTS_RMP 		2402
#define ID_PRINT_2D_PLOT            2403

// Load/Save dialog

#define ID_XSIZE_STR 				   2500
#define ID_YSIZE_STR 				   2501
#define ID_ZSIZE_STR 				   2502
#define ID_QSIZE_STR 				   2503
#define ID_VAR_LBL 				      2504
#define ID_VAR_NAME 				      2505
#define ID_X_VAR_STEXT              2506
#define ID_Y_VAR_STEXT              2507
#define ID_X_VAR_LIST 				   2508
#define ID_Y_VAR_LIST 				   2509
#define ID_USE_XY_STEXT             2510
#define ID_USE_XY                   2511

// Go to line number dialog

#define ID_LINE_NR                  2600

// Object label dialog

#define ID_LABEL_TXT						2700
#define ID_NAME_TXT						2701

// Load 1D plot dialog
 
#define ID_NEW_PLOT						2801
#define ID_INSERT_BEFORE_PLOTS		2802
#define ID_INSERT_AFTER_PLOTS			2803
#define ID_APPEND_PLOTS					2804
#define ID_REPLACE_PLOTS				2805
#define ID_DEFAULT_COLS	   			2806

// Mutiplot dialog

#define MULTIPLOT_X_CELLS           2900
#define MULTIPLOT_Y_CELLS           2901

// Mutiedit dialog

#define MULTIEDIT_X_CELLS           2910
#define MULTIEDIT_Y_CELLS           2911

// Paste 1D data dialog

#define ID_CLEAR_PLOT               3000

// 3D menu

#define ID_ROTATE_CCW_AZIMUTH  	   3100
#define ID_ROTATE_CW_AZIMUTH	      3101
#define ID_ROTATE_CW_ELEVATION  	   3102
#define ID_ROTATE_CCW_ELEVATION	   3103
#define ID_ROTATE_CCW_TWIST  	      3104
#define ID_ROTATE_CW_TWIST	         3105
#define ID_SHIFT_3D_LEFT  	         3106
#define ID_SHIFT_3D_RIGHT	         3107
#define ID_SHIFT_3D_UP  	         3108
#define ID_SHIFT_3D_DOWN	         3109
#define ID_SCALE_X_DOWN             3110
#define ID_SCALE_X_UP               3111
#define ID_SCALE_Y_DOWN             3112
#define ID_SCALE_Y_UP               3113
#define ID_SCALE_Z_DOWN             3114
#define ID_SCALE_Z_UP               3115
#define ID_SHIFT_Z_CUTPLANE_UP      3116
#define ID_SHIFT_Z_CUTPLANE_DOWN    3117
#define ID_CLEAR_3D_PLOT            3118
#define ID_SPECULAR_LIGHTING        3119
#define ID_SMOOTH_SHADING           3120
#define ID_CONTROL_3D_DIALOG        3121
#define ID_COPY_TO_CLIPBOARD        3122   
#define ID_SURFACE_PLOT_PARAMETERS  3123
#define ID_SHIFT_3D_IN  	         3124
#define ID_SHIFT_3D_OUT	            3125
#define ID_RESET_3D_PARAMETERS      3126
#define ID_COPY_SUBPLOT_TO_CLIPBD   3217


// Preferences

#define ID_MENU_NAMES               3200
#define ID_MENU_FOLDERS             3201
#define ID_REMOVE_MENU_NAME         3202
#define ID_ADD_MENU_NAME            3203
#define ID_REMOVE_MENU_FOLDER       3204
#define ID_ADD_MENU_FOLDER          3205
#define ID_MENU_UP                  3206
#define ID_MENU_DOWN                3207
#define ID_SAVE_MENUS               3208               
#define ID_APP_DIR                  3209
#define ID_PREFDIR                  3210
#define ID_ABSOLUTE                 3211
#define ID_SEARCH_PATH              3212
#define ID_REMOVE_PATH              3213
#define ID_ADD_PATH                 3214
#define ID_PATH_UP                  3215
#define ID_PATH_DOWN                3216
#define ID_SAVE_PATH                3217
#define ID_CD_HOME                  3218
#define ID_CD_PROSPA                3219
// Don't insert anything between beginning and end of this block
// unless it is another colour box id.
#define ID_PREF_BG_COLOR            3220
#define ID_PREF_AXES_COLOR          3221
#define ID_PREF_GRID_COLOR          3222
#define ID_PREF_AXES_FONT_COLOR     3223
#define ID_PREF_BORDER_COLOR        3224
#define ID_PREF_FINE_GRID_COLOR     3225
#define ID_PREF_TITLE_COLOR         3226
#define ID_PREF_LABEL_COLOR         3227
#define ID_PREF_REAL_COLOR          3228
#define ID_PREF_IMAG_COLOR          3229
#define ID_PREF_SYMBOL_COLOR        3230
#define ID_PREF_BAR_COLOR           3231

#define ID_PREF_UPDATE_PLOTS        3232
#define ID_PREF_SAVE_PLOT_DEFAULTS  3233
#define ID_PREF_UPDATE_DEFAULTS     3234
#define ID_DISPLAY_WK_DIR           3235
#define ID_SELECT_WK_DIR            3236
#define ID_CACHE_PROC               3237

// Find and replace dialog

#define ID_REPLACE_TEXT_BUTTON          3400
#define ID_FIND_TEXT                    3401
#define ID_REPLACE_TEXT                 3402
#define ID_FIND_TEXT_BUTTON             3403
#define ID_REPLACE_ALL_TEXT_BUTTON      3404
#define ID_FINDREPLACE_IGNORE_CASE      3405

// Color selection dialog template

#define ID_BKGD_COLOR                   3500


// GUI Edit Menu

#define ID_SELECTALL                    3600
#define ID_LEFTALIGN                    3601
#define ID_RIGHTALIGN                   3602
#define ID_ALIGNTOPS                    3603
#define ID_ALIGNBASES                   3604
#define ID_STOPEDITING                  3605
#define ID_MODIFY_CTRL_NUMBERS          3606
#define ID_MODIFY_TAB_NUMBERS           3607
#define ID_TOGGLE_CTRL_NUMBERS          3608
#define ID_TOGGLE_TAB_NUMBERS           3609
#define ID_COPYOBJECTS                  3610
#define ID_PASTEOBJECTS                 3611
#define ID_CUTOBJECTS                   3612
#define ID_ACTIVEWINDOW                 3613

// GUI Run Menu

#define ID_MAKEEDITABLE                 3700
#define ID_TOGGLENUMBERS                3701
#define ID_CLOSE_WINDOW                 3702

// License dialogs
#define LICENSEINFO                     3800
#define ID_REQUEST_LICENSE              3801
#define ID_INSTALL_LICENSE              3802

// Base identity number for user defined macro menus

#define WINDOW_MENU_START      61000 
#define PROC_MENU_START        60000
#define MAIN_UD_MENUS_START    40000
#define OBJECT_UD_MENUS_START  20000
#define MENU_BASE              100
#define MAX_MENU_ITEMS         200

#endif // define PROSPARESOURCE_H

