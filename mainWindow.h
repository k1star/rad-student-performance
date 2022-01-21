//---------------------------------------------------------------------------
#ifndef mainWindowH
#define mainWindowH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.Menus.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Grids.hpp>
#include <Vcl.Dialogs.hpp>
//---------------------------------------------------------------------------
class TMainForm : public TForm
{
__published:	// IDE-managed Components
	TMainMenu *MainMenu;
	TMenuItem *A1;
	TMenuItem *N1;
	TMenuItem *N2;
	TMenuItem *N3;
	TMenuItem *menuAboutProgram;
	TMenuItem *menuCreateNew;
	TMenuItem *menuOpen;
	TMenuItem *menuSave;
	TMenuItem *menuSaveHow;
	TMenuItem *N11;
	TMenuItem *menuExit;
	TMenuItem *menuClearAll;
	TMenuItem *menuFont;
	TMenuItem *menuCorrectWidthCol;
	TMenuItem *N28;
	TMenuItem *menuAutoWidthCol;
	TMenuItem *menuSaveFileResults;
	TStatusBar *StatusBar;
	TPanel *panelWithGrid;
	TPanel *panelWithButtons;
	TStringGrid *sMainGrid;
	TGroupBox *gbStudent;
	TButton *bInsertStudent;
	TButton *bAddStudent;
	TButton *bDeleteStudent;
	TGroupBox *gbSubject;
	TButton *bAddSubject;
	TButton *bInsertSubject;
	TButton *bDeleteSubject;
	TSaveDialog *saveFileDlg;
	TOpenDialog *openFileDlg;
	TFontDialog *FontDlg;
	TGroupBox *gbView;
	TComboBox *cmbBox;
	TCheckBox *chLightSubjects;
	TMenuItem *N5;
	TMenuItem *menuAddStudent;
	TMenuItem *menuDeleteStudent;
	TMenuItem *menuInsertStudent;
	TMenuItem *N10;
	TMenuItem *menuAddSubject;
	TMenuItem *menuInsertSubject;
	TMenuItem *menuDeleteSubject;
	TMenuItem *menuLightSubjects;
	TMenuItem *N4;
	TMenuItem *N6;
	TMenuItem *N7;
	TMenuItem *N8;
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall menuCreateNewClick(TObject *Sender);
	void __fastcall menuSaveClick(TObject *Sender);
	void __fastcall menuSaveHowClick(TObject *Sender);
	void __fastcall menuOpenClick(TObject *Sender);
	void __fastcall bAddStudentClick(TObject *Sender);
	void __fastcall bInsertStudentClick(TObject *Sender);
	void __fastcall bAddSubjectClick(TObject *Sender);
	void __fastcall bInsertSubjectClick(TObject *Sender);
	void __fastcall menuAutoWidthColClick(TObject *Sender);
	void __fastcall menuClearAllClick(TObject *Sender);
	void __fastcall menuFontClick(TObject *Sender);
	void __fastcall FontDlgApply(TObject *Sender, HWND Wnd);
	void __fastcall sMainGridGetEditText(TObject *Sender, int ACol, int ARow, UnicodeString &Value);
	void __fastcall sMainGridSelectCell(TObject *Sender, int ACol, int ARow, bool &CanSelect);
	void __fastcall sMainGridDblClick(TObject *Sender);
	void __fastcall sMainGridKeyPress(TObject *Sender, System::WideChar &Key);
	void __fastcall sMainGridSetEditText(TObject *Sender, int ACol, int ARow, const UnicodeString Value);
	void __fastcall bDeleteStudentClick(TObject *Sender);
	void __fastcall bDeleteSubjectClick(TObject *Sender);
	void __fastcall cmbBoxChange(TObject *Sender);
	void __fastcall sMainGridDrawCell(TObject *Sender, int ACol, int ARow, TRect &Rect,
          TGridDrawState State);
	void __fastcall menuSaveFileResultsClick(TObject *Sender);
	void __fastcall menuAboutProgramClick(TObject *Sender);
	void __fastcall menuLightSubjectsClick(TObject *Sender);
	void __fastcall chLightSubjectsClick(TObject *Sender);
	void __fastcall menuAddStudentClick(TObject *Sender);
	void __fastcall menuInsertStudentClick(TObject *Sender);
	void __fastcall menuDeleteStudentClick(TObject *Sender);
	void __fastcall menuAddSubjectClick(TObject *Sender);
	void __fastcall menuInsertSubjectClick(TObject *Sender);
	void __fastcall menuDeleteSubjectClick(TObject *Sender);
	void __fastcall menuExitClick(TObject *Sender);
	void __fastcall menuCorrectWidthColClick(TObject *Sender);
	void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
	void __fastcall sMainGridRowMoved(TObject *Sender, int FromIndex, int ToIndex);
	void __fastcall sMainGridColumnMoved(TObject *Sender, int FromIndex, int ToIndex);
private:	// User declarations
	void _fastcall SaveDataWithDlgOrNo(bool execute = true);
	void _fastcall DetermineAvgMarks();
    void __fastcall deleteSorted();

	String tempCell;

	int FSortedColNbr;
	int FSortedRowNbr;
public:		// User declarations
	__fastcall TMainForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TMainForm *MainForm;
//---------------------------------------------------------------------------
#endif
