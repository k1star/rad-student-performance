//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "AboutProgram.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfAbout *fAbout;
//---------------------------------------------------------------------------
__fastcall TfAbout::TfAbout(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TfAbout::pWithCloseButtonResize(TObject *Sender)
{
    bClose->Left = pWithCloseButton->Width / 2 - bClose->Width / 2;
	bClose->Top = pWithCloseButton->Height / 2 - bClose->Height / 2;
}
//---------------------------------------------------------------------------
void __fastcall TfAbout::bCloseClick(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------
