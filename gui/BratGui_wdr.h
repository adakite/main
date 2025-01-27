//------------------------------------------------------------------------------
// Header generated by wxDesigner from file: BratGui.wdr
// Do not modify this file, all changes will be lost!
//------------------------------------------------------------------------------

#ifndef __WDR_BratGui_H__
#define __WDR_BratGui_H__

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma interface "BratGui_wdr.h"
#endif

// Include wxWidgets' headers

#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <wx/image.h>
#include <wx/statline.h>
#include <wx/spinbutt.h>
#include <wx/spinctrl.h>
#include <wx/splitter.h>
#include <wx/listctrl.h>
#include <wx/treectrl.h>
#include <wx/notebook.h>
#include <wx/grid.h>
#include <wx/toolbar.h>
#include <wx/tglbtn.h>

// Declare window functions

const int ID_MAINNOTEBOOK = 10000;
wxSizer *MainNotebook( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

const int ID_TEXT = 10001;
const int ID_DSNAMES = 10002;
const int ID_DSNEW = 10003;
const int ID_DSDELETE = 10004;
extern wxSizer *glb_DatasetlistSizer;
const int ID_DSFILELIST = 10005;
const int ID_FILEDESC_LABEL = 10006;
const int ID_FILEDESC = 10007;
const int ID_DSUP = 10008;
const int ID_DSDOWN = 10009;
const int ID_DSSORT = 10010;
const int ID_DSREMOVE = 10011;
const int ID_DSCLEAR = 10012;
const int ID_DSADDFILES = 10013;
const int ID_DSADDDIR = 10014;
const int ID_DSAPPLYCRIT = 10015;
const int ID_DSDEFINECRIT = 10016;
const int ID_DSSHOWREPORT = 10017;
const int ID_DSCHECK = 10018;
wxSizer *DatasetListPanel( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

const int ID_DATASETSPLITTER = 10019;
wxSizer *DatasetSplitPanel( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

const int ID_PRODUCT_LABEL = 10020;
const int ID_DICTLIST = 10021;
const int ID_FIELDDESC = 10022;
wxSizer *DatasetDescCtrl( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

extern wxSizer *glb_WorkspaceDlgSizer;
const int ID_WKS_NAME_STATIC = 10023;
const int ID_WKS_NAME = 10024;
const int ID_WKS_LOC = 10025;
const int ID_WKS_BROWSE = 10026;
extern wxSizer *brathlGuiImportOptionsSizer;
const int ID_IMPORT_DATASET = 10027;
const int ID_IMPORT_FORMULAS = 10028;
const int ID_IMPORT_FORMULAS_SELECT = 10029;
const int ID_IMPORT_OPERATIONS = 10030;
const int ID_IMPORT_VIEWS = 10031;
extern wxSizer *glb_FormulaListToImportSizer;
const int ID_IMPORT_FORMULAS_LIST = 10032;
const int ID_IMPORT_FORMULAS_SELECT_ALL = 10033;
const int ID_IMPORT_FORMULAS_DESELECT_ALL = 10034;
wxSizer *WorkspaceDlg( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

const int ID_FORMULA_NAME = 10035;
const int ID_FORMULA_VALUE = 10036;
const int ID_FORMULA_UNIT = 10037;
const int ID_FORMULA_TITLE = 10038;
const int ID_FORMULA_COMMENT = 10039;
wxSizer *FormulaDlg( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

extern wxSizer *glb_FormulaGlobalSizer;
const int ID_FORMULAOPT_NAME = 10040;
const int ID_FORMULAOPT_TITLE = 10041;
extern wxSizer *glb_FormulaOptFillInSizer;
const int ID_FORMULAOPT_FILL_IN = 10042;
const int ID_FORMULAOPT_FILTER = 10043;
extern wxSizer *glb_FormulaOptIntervalSizer;
const int ID_FORMULAOPT_MIN = 10044;
const int ID_FORMULAOPT_MAX = 10045;
const int ID_FORMULAOPT_HELP_MIN_MAX = 10046;
const int ID_FORMULAOPT_LOESSCUT = 10047;
const int ID_FORMULAOPT_STEP = 10048;
const int ID_FORMULAOPT_HELP_STEP = 10049;
const int ID_FORMULAOPT_INTERVAL = 10050;
const int ID_FORMULAOPT_INTERVAL_WARNING = 10051;
wxSizer *FormulaOptionDlg( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

const int ID_LOGPANEL = 10052;
wxSizer *LogPanel( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

const int ID_TASKLIST = 10053;
const int ID_TASK_KILL = 10054;
const int ID_LOG_MESS = 10055;
const int ID_LOG_CLEAR = 10056;
wxSizer *LogInfoPanel( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

extern wxSizer *glb_OpInfoPanel2Sizer;
const int ID_OPINSERTEXPR = 10057;
const int ID_OPINSERTFIELD = 10058;
const int ID_OPINSERTFCT = 10059;
const int ID_OPINSERTFORMULA = 10060;
const int ID_OPSAVEASFORMULA = 10061;
const int ID_OPINSERTALGORITHM = 10062;
const int ID_OPDELETEEXPR = 10063;
extern wxSizer *glb_OpAllDataExprSizer;
const int ID_OPERATIONTREECTRLLABEL = 10064;
const int ID_OPCHANGERECORD = 10065;
const int ID_OPERATIONTREECTRL = 10066;
extern wxSizer *glb_OpPanelRightPane;
const int ID_OPUNIT = 10067;
const int ID_OPTYPE = 10068;
const int ID_OPXVALUE = 10069;
const int ID_OPTEXTFORM = 10070;
const int ID_OP_DATA_MODE = 10071;
const int ID_OPCHECKDATAF = 10072;
const int ID_OPEXPRINFO = 10073;
const int ID_OPALIASINFO = 10074;
const int ID_OPTITLE = 10075;
extern wxSizer *glb_OpPanelResolutionAndFilterSizer;
extern wxSizer *glb_OpXResolutionSizer;
const int ID_OP_XRESOLUTIONLABEL = 10076;
extern wxSizer *glb_OpYResolutionSizer;
const int ID_OP_YRESOLUTIONLABEL = 10077;
const int ID_OP_FILTERLABEL = 10078;
const int ID_OPRESOLUTION = 10079;
wxSizer *OperationInfoPanel( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

extern wxSizer *glb_OpPanel2Sizer;
const int ID_OPERATIONSPLITTER = 10080;
wxSizer *OperationPanel( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

const int ID_DATASETTREECTRL = 10081;
const int ID_FIELDSTREECTRL = 10082;
wxSizer *DataStruct( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

const int ID_FUNCTION_CAT = 10083;
const int ID_FUNCTION_LIST = 10084;
const int ID_FUNCTION_COMMENT = 10085;
wxSizer *FunctionsDlg( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

const int ID_FORMULA_LIST = 10086;
const int ID_FORMULA_REMOVE = 10087;
const int ID_FORMULA_ALIAS = 10088;
wxSizer *FormulasDlg( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

extern wxSizer *glb_FormulaResolutionSizer;
extern wxSizer *glb_FormulaOptFilterSizer;
extern wxSizer *glb_FormulaOptXIntervalSizer;
const int ID_FORMULAOPT_XMIN = 10089;
const int ID_FORMULAOPT_XMAX = 10090;
const int ID_FORMULAOPT_XGETMINMAX = 10091;
const int ID_FORMULAOPT_HELP_X_MIN_MAX = 10092;
const int ID_FORMULAOPT_XLOESSCUT = 10093;
const int ID_FORMULAOPT_XSTEP = 10094;
const int ID_FORMULAOPT_HELP_X_STEP = 10095;
const int ID_FORMULAOPT_XINTERVAL = 10096;
const int ID_FORMULAOPT_INTERVAL_X_WARNING = 10097;
extern wxSizer *glb_FormulaOptYIntervalSizer;
const int ID_FORMULAOPT_YMIN = 10098;
const int ID_FORMULAOPT_YMAX = 10099;
const int ID_FORMULAOPT_YGETMINMAX = 10100;
const int ID_FORMULAOPT_HELP_Y_MIN_MAX = 10101;
const int ID_FORMULAOPT_YLOESSCUT = 10102;
const int ID_FORMULAOPT_YSTEP = 10103;
const int ID_FORMULAOPT_HELP_Y_STEP = 10104;
const int ID_FORMULAOPT_YINTERVAL = 10105;
const int ID_FORMULAOPT_INTERVAL_Y_WARNING = 10106;
wxSizer *ResolutionDlg( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

const int ID_EXPORTOUTPUTFILE = 10107;
const int ID_EXPORT_BROWSE = 10108;
extern wxSizer *glb_ExportOptionsSizer;
const int ID_EXPORTFORMAT = 10109;
const int ID_DATE_AS_PERIOD = 10110;
const int ID_EXPAND_ARRAY = 10111;
const int ID_EXECAGAIN = 10112;
const int ID_EXPORT_NO_DATA_COMPUTATION = 10113;
const int ID_EXPORT_ASCII_NUMBER_PRECISION = 10114;
extern wxSizer *glb_ExportGeoTiffSizer;
const int ID_EXPORT_CREATE_KML_FILE = 10115;
const int ID_EXPORT_COLOR_TABLE_LABEL = 10116;
const int ID_EXPORT_COLOR_TABLE = 10117;
const int ID_EXPORT_COLORRANGE_MIN = 10118;
const int ID_EXPORT_COLORRANGE_MAX = 10119;
const int ID_EXPORT_CALC_COLOR_RANGE = 10120;
const int ID_EXPORT_DELAY_EXECUTION = 10121;
wxSizer *ExportDlg( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

const int ID_DISPLAYPANEL = 10122;
wxSizer *DisplayPanel( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

extern wxSizer *glbDisplayPanelSizer;
const int ID_DISP_NAMES = 10123;
const int ID_DISPEXECUTE = 10124;
const int ID_DISPNEW = 10125;
const int ID_DISPDELETE = 10126;
extern wxSizer *glbDisplayGeneralPropsSizer;
const int ID_DISP_TITLE = 10127;
const int ID_DISP_WITH_ANIMATION = 10128;
const int ID_DISP_PROJ = 10129;
extern wxSizer *glbDisplayZoomSizer;
extern wxSizer *glbDisplayZoomGeo;
const int ID_DISPZOOM_LON_WEST = 10130;
const int ID_DISPZOOM_LAT_NORTH = 10131;
const int ID_DISPZOOM_LAT_SOUTH = 10132;
const int ID_DISPZOOM_LON_EAST = 10133;
extern wxSizer *glbDisplayZoomXY;
const int ID_DISP_XMIN = 10134;
const int ID_DISP_XMAX = 10135;
const int ID_DISP_YMIN = 10136;
const int ID_DISP_YMAX = 10137;
const int ID_DISPAVAILTREECTRL = 10138;
const int ID_DISP_REFRESH = 10139;
const int ID_DISP_ADDFIELD = 10140;
const int ID_DISP_REMOVEFIELD = 10141;
extern wxSizer *glbDisplaySelected;
const int ID_DISPGROUPFIELDS = 10142;
const int ID_DISP_TYPE = 10143;
const int ID_DISP_DATA_SEL = 10144;
extern wxSizer *glbDisplayFieldPropsAll;
extern wxSizer *glbDisplayFieldPropsAll2;
extern wxSizer *glbDisplayFieldProps1;
const int ID_DISP_FIELD_NAME = 10145;
extern wxSizer *glbDisplayFieldPropsZFXY1;
const int ID_DISP_MIN = 10146;
const int ID_DISP_MAX = 10147;
extern wxSizer *glbDisplayFieldPropsZFXY2;
const int ID_DISP_NORTH_COMPONENT = 10148;
const int ID_DISP_EAST_COMPONENT = 10149;
const int ID_DISP_SOLID_COLOR = 10150;
const int ID_DISP_CONTOUR = 10151;
const int ID_DISP_PALETTE_LABEL = 10152;
const int ID_DISP_PALETTE = 10153;
const int ID_DISP_PAL_BROWSE = 10154;
const int ID_DISP_INVERT_XYAXES = 10155;
extern wxSizer *glbDisplayFieldPropsYFX1;
const int ID_DISP_XAXIS_LABEL = 10156;
const int ID_DISP_XAXIS = 10157;
const int ID_DISP_XAXIS_TEXT_LABEL = 10158;
const int ID_DISP_XAXIS_TEXT = 10159;
wxSizer *DisplayInfoPanel( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

const int ID_RECORD_LIST = 10160;
wxSizer *RecordDlg( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

const int ID_OPNAMES = 10161;
const int ID_OPEXECUTE = 10162;
const int ID_OPNEW = 10163;
const int ID_OPDUP = 10164;
const int ID_OPDELETE = 10165;
const int ID_OPEXPORT = 10166;
const int ID_OPEXPORTASCIIEDIT = 10167;
const int ID_OPSHOWSTATS = 10168;
const int ID_OPDELAY = 10169;
const int ID_LAUNCH_SCHEDULER = 10170;
wxSizer *OperationPanelHeader( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

extern wxSizer *glbSelectionCriteriaDlgSizer;
const int ID_CRIT_PRODUCT_LIST = 10171;
extern wxSizer *glbCritSizer;
extern wxSizer *glbCritDateSizer;
const int ID_CRIT_DATE_MIN = 10172;
const int ID_CRIT_DATE_MAX = 10173;
extern wxSizer *glbCritLatLonSizer;
const int ID_CRIT_LON_WEST = 10174;
const int ID_CRIT_LAT_NORTH = 10175;
const int ID_CRIT_LAT_SOUTH = 10176;
const int ID_CRIT_LON_EAST = 10177;
extern wxSizer *glbCritCycleFromToSizer;
const int ID_CRIT_CYCLE_FROM = 10178;
const int ID_CRIT_CYCLE_TO = 10179;
extern wxSizer *glbCritPassFromToSizer;
const int ID_CRIT_PASS_FROM = 10180;
const int ID_CRIT_PASS_TO = 10181;
extern wxSizer *glbCritPassListSizer;
const int ID_CRIT_PASS_LIST = 10182;
wxSizer *SelectionCriteriaDlg( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

extern wxSizer *glbExprInfoGridSizerH;
extern wxSizer *glbExprInfoGridSizerV;
const int ID_EXPRINFO_GRID = 10183;
const int ID_EXPRINFO_RESULT_UNIT = 10184;
wxSizer *ExpressionInfoDlg( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

const int ID_UNITFIELD_FIELD = 10185;
const int ID_UNITFIELD_ACTUALUNIT = 10186;
const int ID_UNITFIELD_NEWUNIT = 10187;
wxSizer *UnitFieldDlg( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

const int ID_ALGO_LIST = 10188;
const int ID_ALGO_DESCR = 10189;
extern wxSizer *glbAlgoInputParamSizerV;
extern wxSizer *glbAlgoInputParamSizerH;
const int ID_ALGO_INPUT_PARAM_GRID = 10190;
const int ID_ALGO_OUTPUT_UNIT = 10191;
wxSizer *AlgorithmDlg( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

const int ID_DATE_PICKER = 10192;
const int ID_LINE = 10193;
const int ID_TIME_PICKER = 10194;
const int ID_TASK_NAME = 10195;
wxSizer *DelayDlg( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

const int ID_ALIASINFO_HEADER = 10196;
extern wxSizer *glbAliasParamSizerH;
extern wxSizer *glbAliasParamSizerV;
const int ID_ALIASINFO_GRID = 10197;
const int ID_ALIASINFO_FOOTER = 10198;
wxSizer *AliasInfoDlg( wxWindow *parent, bool call_fit = TRUE, bool set_sizer = TRUE );

// Declare menubar functions

const int ID_WSMENU = 10199;
const int ID_WSNEW = 10200;
const int ID_WSOPEN = 10201;
const int ID_WSSAVE = 10202;
const int ID_WSIMPORT = 10203;
const int ID_WSRENAME = 10204;
const int ID_WSDELETE = 10205;
const int ID_MENU = 10206;
const int ID_MENU_FILE_RECENT = 10207;
const int ID_DSMENU = 10208;
const int ID_OPMENU = 10209;
const int ID_OPDUPLICATE = 10210;
const int ID_OPVIEW = 10211;
const int ID_VINEW = 10212;
const int ID_VIDELETE = 10213;
const int ID_HELPMENU = 10214;
const int ID_HELP_USER_MANUEL = 10215;
wxMenuBar *MainMenuBarFunc();

const int ID_FIELDCTRLMENU = 10216;
const int ID_ASXMENU = 10217;
const int ID_ASYMENU = 10218;
const int ID_ASDATAMENU = 10219;
const int ID_ASSELECTMENU = 10220;
const int ID_FIELDSSORTMENU = 10221;
const int ID_FIELDSSORTREVMENU = 10222;
const int ID_FIELD_ATTR_CHANGE_UNIT = 10223;
const int ID_OPTREECTRLMENU = 10224;
const int ID_INSERTEXPRMENU = 10225;
const int ID_INSERTFIELDMENU = 10226;
const int ID_INSERTFCTMENU = 10227;
const int ID_INSERTFORMULAMENU = 10228;
const int ID_SAVEASFORMULAMENU = 10229;
const int ID_DELETEEXPRMENU = 10230;
const int ID_RENAMEEXPRMENU = 10231;
const int ID_OPSORTMENU = 10232;
const int ID_OPSORTREVMENU = 10233;
const int ID_DISPTREECTRLMENU = 10234;
const int ID_DISPADDTOSEL = 10235;
const int ID_DISPEXPANDMENU = 10236;
const int ID_DISPCOLLAPSEMENU = 10237;
const int ID_DISPSORTMENU = 10238;
const int ID_DISPSORTREVMENU = 10239;
wxMenuBar *PopupMenuFunc();

// Declare toolbar functions

const int ID_TOOL = 10240;
void MainToolBarFunc( wxToolBar *parent );

// Declare bitmap functions

wxBitmap ButtonBitmapsFunc( size_t index );

wxBitmap BitmapsList( size_t index );

wxBitmap TreeIconsDefault( size_t index );

#endif

// End of generated file
