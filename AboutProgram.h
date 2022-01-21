//---------------------------------------------------------------------------
#ifndef AboutProgramH
#define AboutProgramH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TfAbout : public TForm
{
__published:	// IDE-managed Components
	TRichEdit *rEdit;
	TPanel *pWithCloseButton;
	TButton *bClose;
	void __fastcall pWithCloseButtonResize(TObject *Sender);
	void __fastcall bCloseClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
	__fastcall TfAbout(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TfAbout *fAbout;
//---------------------------------------------------------------------------
#endif
