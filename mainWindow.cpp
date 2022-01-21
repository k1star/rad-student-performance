// ---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "mainWindow.h"
#include "AboutProgram.h"
#include <boost/crc.hpp>
#include <fstream>
#include <Math.hpp>
#include <math.h>
#include <iomanip>
// ---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMainForm *MainForm;

// ---------------------------------------------------------------------------
__fastcall TMainForm::TMainForm(TComponent* Owner) : TForm(Owner) {
}

// ---------------------------------------------------------------------------
class TAdditionalSGrid : TStringGrid {
public:
	// virtual void __fastcall DeleteRow(int ARow);
	using TStringGrid::DeleteRow;

	using TStringGrid::DeleteColumn;

	using TStringGrid::MoveColumn;

	using TStringGrid::MoveRow;
};

// ---------------------------------------------------------------------------
int GetMaxColWidth(TStringGrid *grid, int colIdx) {
	if (!grid)
		return -1;

	grid->Canvas->Font = grid->Font;

	int maxWidth = 0;

	for (int j = 0; j < grid->RowCount; j++) {
		if (grid->Canvas->TextWidth(grid->Cells[colIdx][j]) > maxWidth)
			maxWidth = grid->Canvas->TextWidth(grid->Cells[colIdx][j]);
	}
	return maxWidth;
}

// ---------------------------------------------------------------------------
void AutoSizeGrid(TStringGrid *grid) {
	// grid->Repaint(); Слишком низкая эффективность, перерисовывает весь грид

	for (int i = 0; i < grid->ColCount; i++) {
		int maxWidth = GetMaxColWidth(grid, i);

		if (grid->ColWidths[i] != -1) {
			if (maxWidth > grid->DefaultColWidth)
				grid->ColWidths[i] = maxWidth + 12;
			else
				grid->ColWidths[i] = grid->DefaultColWidth + 12;
		}
	}
}

// ---------------------------------------------------------------------------
void _fastcall TMainForm::DetermineAvgMarks() {
	if (sMainGrid->RowHeights[1] == -1 && sMainGrid->ColWidths[1] == -1)
		return;

	sMainGrid->Cells[0][sMainGrid->RowCount - 1] = L"Средний балл (предмет)";

	sMainGrid->Cells[sMainGrid->ColCount - 1][0] = L"Средний балл (студент)";

	float determineAvg, tempFloat;
	int numberExisting;
	for (int ACol = 1; ACol < sMainGrid->ColCount - 1; ACol++) {
		determineAvg = 0;
		numberExisting = 0;

		for (int ARow = 1; ARow < sMainGrid->RowCount - 1; ARow++)
			if (sMainGrid->Cells[ACol][ARow] != L"" && sMainGrid->RowHeights
				[ARow] != -1) {
				if (!TryStrToFloat(sMainGrid->Cells[ACol][ARow], tempFloat)) {
					determineAvg = 0;
					break;
				}
				else {
					determineAvg += tempFloat;
					numberExisting++;
				}
			}
		if (determineAvg == 0) {
			sMainGrid->Cells[ACol][sMainGrid->RowCount - 1] = L"-";
			continue;
		}

		determineAvg /= numberExisting;
		sMainGrid->Cells[ACol][sMainGrid->RowCount - 1] =
			String(FormatFloat("0.0", determineAvg));
	}

	for (int ARow = 1; ARow < sMainGrid->RowCount - 1; ARow++) {
		if (sMainGrid->RowHeights[ARow] == -1)
			continue;

		determineAvg = 0;
		numberExisting = 0;

		for (int ACol = 1; ACol < sMainGrid->ColCount - 1; ACol++)
			if (sMainGrid->Cells[ACol][ARow] != L"") {
				if (!TryStrToFloat(sMainGrid->Cells[ACol][ARow], tempFloat)) {
					determineAvg = 0;
					break;
				}
				else {
					determineAvg += StrToFloat(sMainGrid->Cells[ACol][ARow]);
					numberExisting++;
				}
			}
		if (determineAvg == 0) {
			sMainGrid->Cells[sMainGrid->ColCount - 1][ARow] = L"-";
			continue;
		}

		determineAvg /= numberExisting;
		sMainGrid->Cells[sMainGrid->ColCount - 1][ARow] =
			String(FormatFloat("0.0", determineAvg));
	}

	sMainGrid->Cells[sMainGrid->ColCount - 1][sMainGrid->RowCount - 1] = L"";
}

// ---------------------------------------------------------------------------
typedef float TCmpFunc(const UnicodeString & s1, const UnicodeString & s2);

float cmpFuncText(const UnicodeString & s1, const UnicodeString & s2) {
	// return AnsiCompareStr(s1, s2);//регистро-зависимое
	return AnsiCompareText(s1, s2); // не зависит от регистра
}

float cmpFuncIntNumbers(const UnicodeString & a, const UnicodeString & b) {
	if (a == L"")
		return -1;
	if (b == L"")
		return 1;
	return StrToInt(a) - StrToInt(b);
}

float cmpFuncFloatNumbers(const UnicodeString & a, const UnicodeString & b) {
	if (a == L"" || a == L"-")
		return -1;
	if (b == L"" || b == L"-")
		return 1;
	return StrToFloat(a) - StrToFloat(b);
}

// ---------------------------------------------------------------------------
void SortGridByCol(TStringGrid * grid, int colIdx, bool ascending,
	TCmpFunc cmpFunc) {
	TStrings *col = grid->Cols[colIdx];
	TStrings *tmpRow = new TStringList();

	try {
		for (int i = grid->FixedRows; i < grid->RowCount - 2; i++)
			for (int j = i + 1; j < grid->RowCount - 1; j++)
				if (cmpFunc(col->Strings[i], col->Strings[j]) >
					0 && ascending || cmpFunc(col->Strings[i],
					col->Strings[j]) < 0 && !ascending) {
					tmpRow->Assign(grid->Rows[i]);
					grid->Rows[i]->Assign(grid->Rows[j]);
					grid->Rows[j]->Assign(tmpRow);
				}
	}
	__finally {
		delete tmpRow;
	}
}

// ---------------------------------------------------------------------------
void SortGridByRow(TStringGrid * grid, int rowIdx, bool ascending,
	TCmpFunc cmpFunc) {
	TStrings *row = grid->Rows[rowIdx];
	TStrings *tmpCol = new TStringList();

	try {
		for (int i = grid->FixedCols; i < grid->ColCount - 2; i++)
			for (int j = i + 1; j < grid->ColCount - 1; j++)
				if (cmpFunc(row->Strings[i], row->Strings[j]) >
					0 && ascending || cmpFunc(row->Strings[i],
					row->Strings[j]) < 0 && !ascending) {
					tmpCol->Assign(grid->Cols[i]);
					grid->Cols[i]->Assign(grid->Cols[j]);
					grid->Cols[j]->Assign(tmpCol);
				}
	}
	__finally {
		delete tmpCol;
	}
}

// ---------------------------------------------------------------------------
bool isColMaxMarksFive(TStringGrid * grid, int &y, int &x) {
	int maxFives, max = 0;
	int resColIdx = -1, resRowIdx = -1;

	for (int colIdx = 1; colIdx < grid->ColCount - 1; colIdx++) {
		maxFives = 0;
		for (int rowIdx = 1; rowIdx < grid->RowCount - 1; rowIdx++)
			if (grid->Cells[colIdx][rowIdx] == L"5")
				maxFives++;
		if (maxFives > max) {
			max = maxFives;
			resColIdx = colIdx;
			resRowIdx = 0;
		}
	}

	if (y == resColIdx && x == resRowIdx && max != 0)
		return true;
	else
		return false;
}

// ---------------------------------------------------------------------------
bool isColMaxMarksTwo(TStringGrid * grid, int &y, int &x) {
	int maxTwo, max = 0;
	int resColIdx = -1, resRowIdx = -1;

	for (int colIdx = 1; colIdx < grid->ColCount - 1; colIdx++) {
		maxTwo = 0;
		for (int rowIdx = 1; rowIdx < grid->RowCount - 1; rowIdx++)
			if (grid->Cells[colIdx][rowIdx] == L"2")
				maxTwo++;
		if (maxTwo > max) {
			max = maxTwo;
			resColIdx = colIdx;
			resRowIdx = 0;
		}
	}

	if (y == resColIdx && x == resRowIdx && max != 0)
		return true;
	else
		return false;
}

// ---------------------------------------------------------------------------
void __fastcall TMainForm::deleteSorted() {
	int k = abs(FSortedColNbr) - 1;
	int i = abs(FSortedRowNbr) - 1;

	if (FSortedColNbr)
		sMainGrid->Cells[k][0] = sMainGrid->Cells[k][0].SubString(1,
		sMainGrid->Cells[k][0].Length() - 2);
	FSortedColNbr = 0;

	if (FSortedRowNbr)
		sMainGrid->Cells[0][i] = sMainGrid->Cells[0][i].SubString(1,
		sMainGrid->Cells[0][i].Length() - 2);
	FSortedRowNbr = 0;
}

// ---------------------------------------------------------------------------
void _fastcall TMainForm::SaveDataWithDlgOrNo(bool execute) { // Процед. Сохр
	saveFileDlg->Filter =
		"Файлы с данными о успеваемости студентов (*.bff)|*.BFF|Все файлы (*.*)|*.*";
    saveFileDlg->DefaultExt = L"bff";

	if (execute) {
		if (!saveFileDlg->Execute(this->Handle))
			return;
	}

	fstream file;
	file.open(AnsiString(saveFileDlg->FileName).c_str(),
		ios::out | ios::binary);
	if (!file.is_open()) {
		Application->MessageBox
			((String(L"Не удалось создать файл \"") + saveFileDlg->FileName +
			String(L"\"!")).w_str(), Application->Title.w_str(),
			MB_OK | MB_ICONERROR);
		return;
	}

	int amountSubjects, amountStudents;
	int strSize;
	AnsiString tempStr;
	char signature[4] = "ZXC";

	amountSubjects = sMainGrid->ColCount - 2;
	amountStudents = sMainGrid->RowCount - 2;

	file.write((char*)&signature, sizeof(int));
	file.seekp(16);

	file.write((char*)&amountSubjects, sizeof(int));
	file.write((char*)&amountStudents, sizeof(int));

	// Запись названий предметов
	for (int i = 0; i < amountSubjects; i++) {
		if (sMainGrid->Cells[i + 1][0] != L"") {
			tempStr = sMainGrid->Cells[i + 1][0];

			if (abs(FSortedColNbr) - 1 == i + 1)
				tempStr = tempStr.SubString(1, tempStr.Length() - 2);

			strSize = tempStr.Length() + 1;
			file.write((char*)&strSize, sizeof(int));
			file.write((char*)tempStr.c_str(), strSize);
		}
		else {
			strSize = 0;
			file.write((char*)&strSize, sizeof(int));
		}

	}

	// Запись фамилий
	for (int i = 0; i < amountStudents; i++) {
		if (sMainGrid->Cells[0][i + 1] != L"") {
			tempStr = sMainGrid->Cells[0][i + 1];

			if (abs(FSortedRowNbr) - 1 == i + 1)
				tempStr = tempStr.SubString(1, tempStr.Length() - 2);

			strSize = tempStr.Length() + 1;
			file.write((char*)&strSize, sizeof(int));
			file.write((char*)tempStr.c_str(), strSize);
		}
		else {
			strSize = 0;
			file.write((char*)&strSize, sizeof(int));
		}
	}

	// Запись оценок//
	for (int i = 0; i < amountStudents; i++)
		for (int j = 0; j < amountSubjects; j++) {
			if (sMainGrid->Cells[j + 1][i + 1] == L"") {
				strSize = -1;
				file.write((char*)&strSize, sizeof(int));
				continue;
			}

			if (!TryStrToInt(sMainGrid->Cells[j + 1][i + 1], strSize)) {
				StatusBar->SimpleText =
					String(L"❌ Ошибка! Некорректное значение") +
					String(L" оценки в ячейке таблицы [") + (j + 2) +
					String(L"][") + (i + 2) + String(L"]");

				Application->MessageBox
					((String(L"Некорректное значение") +
					String(L" оценки в ячейке таблицы [") + (j + 2) +
					String(L"][") + (i + 2) + String(L"]")).w_str(),
					Application->Title.w_str(), MB_OK | MB_ICONERROR);
				return;
			}
			file.write((char*)&strSize, sizeof(int));
		}

	file.close();

	file.open(AnsiString(saveFileDlg->FileName).c_str(), ios::in, ios::binary);
	std::streamsize fileSize, bufferSize;
	file.seekg(0, ios::end);
	fileSize = file.tellg();

	boost::crc_basic<32>crc(0x27809EA7, 0u, 0u, true, true); {
		bufferSize = fileSize - 16;
		char *buffer = new(std::nothrow) char[bufferSize];

		file.seekg(16, ios::beg);
		file.read((char *)buffer, bufferSize);
		crc.process_bytes(buffer, (size_t)bufferSize);
		delete[]buffer;
	} file.close();

	file.open(AnsiString(saveFileDlg->FileName).c_str(),
		ios::in | ios::out | ios::binary);
	unsigned int hash = crc.checksum();
	file.seekp(4);
	file.write((char*)&fileSize, sizeof(std::streamsize));
	file.write((char*)&hash, sizeof(hash));

	file.flush();
	file.close();

	menuSave->Enabled = false;

	MainForm->Caption = Application->Title + L" - " + saveFileDlg->FileName;
	StatusBar->SimpleText = String(L"✅ Данные успешно сохранены в файле!");
}

// ---------------------------------------------------------------------------
void __fastcall TMainForm::FormCreate(TObject * Sender) { // Создание формы
	sMainGrid->RowCount = 2;
	sMainGrid->ColCount = 2;

	sMainGrid->FixedRows = 1;
	sMainGrid->FixedCols = 1;

	// Скрытие строк для обозначения "нет данных"
	sMainGrid->RowHeights[1] = -1;
	sMainGrid->ColWidths[1] = -1;

	sMainGrid->Cells[0][0] = L"Студент \\ Предмет";

	AutoSizeGrid(sMainGrid);
	menuSave->Enabled = false;

	saveFileDlg->FileName = L"Безымянный";
	openFileDlg->FileName = L"Безымянный";

	FSortedColNbr = 0;
	FSortedRowNbr = 0;

	bDeleteStudent->Enabled = false;
	bDeleteSubject->Enabled = false;
	menuDeleteStudent->Enabled = false;
	menuDeleteSubject->Enabled = false;

	MainForm->Caption = Application->Title + L" - " + saveFileDlg->FileName;
}

// ---------------------------------------------------------------------------
void __fastcall TMainForm::menuCreateNewClick(TObject * Sender) { // Новый файл
	if (menuSave->Enabled)
		switch (Application->MessageBox
			((String(L"Сохранить изменения в файле \"") +
			saveFileDlg->FileName + "\"?").w_str(), Application->Title.w_str(),
			MB_YESNOCANCEL | MB_ICONQUESTION)) {
		case mrYes: {
				menuSaveClick(Sender);
				break;
			}
		case mrCancel:
			return;
		case mrNo: ;
		}

	// Очистка таблицы
	for (int i = 0; i < sMainGrid->RowCount; i++)
		sMainGrid->Rows[i]->Clear();

	sMainGrid->RowCount = 2;
	sMainGrid->ColCount = 2;

	// Скрытие строк для обозначения "нет данных"
	sMainGrid->RowHeights[1] = -1;
	sMainGrid->ColWidths[1] = -1;

	sMainGrid->Cells[0][0] = L"Студент \\ Предмет";

	if (menuAutoWidthCol->Checked)
		AutoSizeGrid(sMainGrid);

	saveFileDlg->FileName = L"Безымянный";
	openFileDlg->FileName = L"Безымянный";

	FSortedColNbr = 0;
	FSortedRowNbr = 0;

	bDeleteStudent->Enabled = false;
	bDeleteSubject->Enabled = false;
	menuDeleteStudent->Enabled = false;
	menuDeleteSubject->Enabled = false;

	menuSave->Enabled = false;
	MainForm->Caption = Application->Title + L" - " + openFileDlg->FileName;
	StatusBar->SimpleText = L"✅ Готово к работе";
}

// ---------------------------------------------------------------------------
void __fastcall TMainForm::menuSaveClick(TObject * Sender) // Сохранить
{
	if (saveFileDlg->FileName == L"Безымянный")
		SaveDataWithDlgOrNo(true);
	else
		SaveDataWithDlgOrNo(false);
}

// ---------------------------------------------------------------------------
void __fastcall TMainForm::menuSaveHowClick(TObject * Sender)
	// Сохранить как
{
	SaveDataWithDlgOrNo(true);
}
// ---------------------------------------------------------------------------

void __fastcall TMainForm::menuOpenClick(TObject * Sender) { // Открыть
	if (menuSave->Enabled == true) {
		switch (Application->MessageBox
			((String(L"Сохранить изменения в файле \"") +
			openFileDlg->FileName + "\"?").w_str(), Application->Title.w_str(),
			MB_YESNOCANCEL | MB_ICONQUESTION)) {
		case mrYes: {
				if (saveFileDlg->FileName == L"Безымянный")
					SaveDataWithDlgOrNo(true);
				else
					SaveDataWithDlgOrNo(false);
				break;
			}
		case mrCancel:
			return;
		case mrNo: ;
		}
	}

	if (!openFileDlg->Execute(this->Handle))
		return;

	fstream file;
	AnsiString oldFileName = openFileDlg->FileName;
	file.open(AnsiString(openFileDlg->FileName).c_str(), ios::in | ios::binary);
	if (!file.is_open()) {
		Application->MessageBox
			((String(L"Не удалось открыть файл \"") + openFileDlg->FileName +
			String(L"\"!")).w_str(), Application->Title.w_str(),
			MB_OK | MB_ICONERROR);
		StatusBar->SimpleText = String(L"❌ Ошибка! Не удалось открыть файл");
		openFileDlg->FileName = oldFileName;
		return;
	}

	int amountSubjects, amountStudents;
	int strSize;
	AnsiString tempStr;

	char signature[4];
	file.read((char*)&signature, sizeof(signature));
	if (strcmp("ZXC", signature)) // Проверка сигнатуры
		switch (Application->MessageBox((String(L"Сигнатура файла \"") +
			openFileDlg->FileName + String(L"\" не совпадает с") +
			String(L" сигнатурой файлов, создаваемых программой.") +
			String(L"\nВсё равно открыть?")).w_str(),
			Application->Title.w_str(), MB_YESNO | MB_ICONQUESTION)) {
		case mrYes: {
				break;
			}
		case mrNo: {
				openFileDlg->FileName = oldFileName;
				file.close();
				return;
			}
		}

	oldFileName = L"";
	saveFileDlg->FileName = openFileDlg->FileName;

	std::streamsize fileSize, bufferSize;
	file.read((char*)&fileSize, sizeof(fileSize)); {
		file.seekg(0, ios::end);
		std::streamsize realFileSize = file.tellg();

		if (realFileSize != fileSize) {
			file.close();
			Application->MessageBox
				((String(L"Нарушена целостность файла \"") +
				openFileDlg->FileName + String(L"\"!")).w_str(),
				Application->Title.w_str(), MB_OK | MB_ICONERROR);
			StatusBar->SimpleText =
				String(L"❌ Ошибка! Не удалось открыть файл");
			return;
		}
	} file.seekg(12);
	unsigned int hash;
	file.read((char*)&hash, sizeof(hash));

	bufferSize = fileSize - 16; {
		char *buffer = new(std::nothrow) char[bufferSize];
		boost::crc_basic<32>crc(0x27809EA7, 0u, 0u, true, true);
		file.seekg(16);

		file.read((char *)buffer, bufferSize);
		crc.process_bytes(buffer, (size_t)bufferSize);

		delete[]buffer;

		if (hash != crc.checksum()) {
			file.close();
			Application->MessageBox
				((String(L"Нарушена целостность файла \"") +
				openFileDlg->FileName + String(L"\"!")).w_str(),
				Application->Title.w_str(), MB_OK | MB_ICONERROR);
			StatusBar->SimpleText =
				String(L"❌ Ошибка! Не удалось открыть файл");
			return;
		}
	}

	file.seekg(16);

	file.read((char*)&amountSubjects, sizeof(amountSubjects));
	file.read((char*)&amountStudents, sizeof(amountStudents));

	// Работа с признаками - нет данных
	sMainGrid->RowHeights[1] = amountStudents != 0 ?
		sMainGrid->DefaultRowHeight : -1;
	sMainGrid->ColWidths[1] = amountSubjects != 0 ?
		sMainGrid->DefaultColWidth : -1;

	// Очистка таблицы
	for (int i = 0; i < sMainGrid->RowCount; i++)
		sMainGrid->Rows[i]->Clear();
	sMainGrid->Cells[0][0] = L"Фамилия \\ Предмет";

	sMainGrid->ColCount = amountSubjects + 2;
	sMainGrid->RowCount = amountStudents + 2;

	// Занесение названий предметов в поля sMainGrid
	for (int i = 0; i < amountSubjects; i++) {
		file.read((char*)&strSize, sizeof(int));
		if (strSize != 0) {
			file.read((char*)tempStr.c_str(), strSize);
			sMainGrid->Cells[i + 1][0] = tempStr.c_str();
		}
		else
			sMainGrid->Cells[i + 1][0] = L"";
	}

	// Занесение фамилий в поля sMainGrid
	for (int i = 0; i < amountStudents; i++) {
		file.read((char*)&strSize, sizeof(int));
		if (strSize != 0) {
			file.read((char*)tempStr.c_str(), strSize);
			sMainGrid->Cells[0][i + 1] = tempStr.c_str();
		}
		else
			sMainGrid->Cells[0][i + 1] = L"";
	}

	// Занесения оценок в поля sMainGrid
	for (int i = 0; i < amountStudents; i++)
		for (int j = 0; j < amountSubjects; j++) {
			file.read((char*)&strSize, sizeof(int));
			if (strSize == -1)
				sMainGrid->Cells[j + 1][i + 1] = L"";
			else
				sMainGrid->Cells[j + 1][i + 1] = IntToStr(strSize);
		} // ПРОВЕРИТЬ МЕСТА КОЛОННЫ И СТРОКИ

	file.close();

	menuSave->Enabled = false;

	FSortedColNbr = 0;
	FSortedRowNbr = 0;

	bDeleteStudent->Enabled = true;
	bDeleteSubject->Enabled = true;
	menuDeleteStudent->Enabled = true;
	menuDeleteSubject->Enabled = true;

	MainForm->Caption = Application->Title + L" - " + openFileDlg->FileName;
	StatusBar->SimpleText = String(L"✅ Файл успешно открыт");

	DetermineAvgMarks();

	if (menuAutoWidthCol->Checked)
		AutoSizeGrid(sMainGrid);
}

// ---------------------------------------------------------------------------
void __fastcall TMainForm::bAddStudentClick(TObject * Sender) {
	if (sMainGrid->RowHeights[1] == -1) { // если нет строк с данными
		sMainGrid->RowHeights[1] = sMainGrid->DefaultRowHeight;
		if (sMainGrid->ColWidths[1] == -1)
			sMainGrid->ColWidths[1] = sMainGrid->DefaultColWidth;
		sMainGrid->RowCount++;
	}
	else {
		sMainGrid->Rows[sMainGrid->RowCount - 1]->Clear();
		sMainGrid->Row = sMainGrid->RowCount - 1;
		sMainGrid->RowCount++;
		sMainGrid->SetFocus();
	}

	DetermineAvgMarks();
	if (menuAutoWidthCol->Checked)
		AutoSizeGrid(sMainGrid);

	bDeleteStudent->Enabled = true;
	menuDeleteStudent->Enabled = true;

	if (MainForm->Caption.SubString(MainForm->Caption.Length(), 1) != L"*")
		MainForm->Caption = MainForm->Caption + L"*";
	menuSave->Enabled = true;
}
// ---------------------------------------------------------------------------

void __fastcall TMainForm::bInsertStudentClick(TObject * Sender) {
	if (sMainGrid->RowHeights[1] == -1) { // если нет строк с данными
		sMainGrid->RowHeights[1] = sMainGrid->DefaultRowHeight;
		if (sMainGrid->ColWidths[1] == -1)
			sMainGrid->ColWidths[1] = sMainGrid->DefaultColWidth;
		sMainGrid->RowCount++;
	}
	else {
		panelWithGrid->Tag = 1;
		sMainGrid->RowCount++;
		((TAdditionalSGrid*)sMainGrid)->MoveRow(sMainGrid->RowCount - 1,
			sMainGrid->Row);
		sMainGrid->Rows[sMainGrid->Row - 1]->Clear();
		sMainGrid->Row--;
	}

	DetermineAvgMarks();

	if (menuAutoWidthCol->Checked)
		AutoSizeGrid(sMainGrid);

	bDeleteStudent->Enabled = true;
	menuDeleteStudent->Enabled = true;

	if (MainForm->Caption.SubString(MainForm->Caption.Length(), 1) != L"*")
		MainForm->Caption = MainForm->Caption + L"*";
	menuSave->Enabled = true;
}

// ---------------------------------------------------------------------------
void __fastcall TMainForm::bAddSubjectClick(TObject * Sender) {
	if (sMainGrid->ColWidths[1] == -1) { // если нет столбцов с данными
		sMainGrid->ColWidths[1] = sMainGrid->DefaultColWidth;
		if (sMainGrid->RowHeights[1] == -1)
			sMainGrid->RowHeights[1] = sMainGrid->DefaultRowHeight;
		sMainGrid->ColCount++;
	}
	else {
		sMainGrid->Cols[sMainGrid->ColCount - 1]->Clear();
		sMainGrid->Col = sMainGrid->ColCount - 1;
		sMainGrid->ColCount++;
		sMainGrid->SetFocus();
	}

	DetermineAvgMarks();
	if (menuAutoWidthCol->Checked)
		AutoSizeGrid(sMainGrid);

	bDeleteSubject->Enabled = true;
	menuDeleteSubject->Enabled = true;

	if (MainForm->Caption.SubString(MainForm->Caption.Length(), 1) != L"*")
		MainForm->Caption = MainForm->Caption + L"*";
	menuSave->Enabled = true;
}
// ---------------------------------------------------------------------------

void __fastcall TMainForm::bInsertSubjectClick(TObject * Sender) {
	if (sMainGrid->ColWidths[1] == -1) { // если нет строк с данными
		sMainGrid->ColWidths[1] = sMainGrid->DefaultColWidth;
		if (sMainGrid->RowHeights[1] == -1)
			sMainGrid->RowHeights[1] = sMainGrid->DefaultRowHeight;
		sMainGrid->ColCount++;
	}
	else {
		panelWithButtons->Tag = 1;
		sMainGrid->ColCount++;
		((TAdditionalSGrid*)sMainGrid)->MoveColumn(sMainGrid->ColCount - 1,
			sMainGrid->Col);
		sMainGrid->Cols[sMainGrid->Col - 1]->Clear();
		sMainGrid->Col--;
	}

	DetermineAvgMarks();

	if (menuAutoWidthCol->Checked)
		AutoSizeGrid(sMainGrid);

	bDeleteSubject->Enabled = true;
	menuDeleteSubject->Enabled = true;

	if (MainForm->Caption.SubString(MainForm->Caption.Length(), 1) != L"*")
		MainForm->Caption = MainForm->Caption + L"*";
	menuSave->Enabled = true;
}

// ---------------------------------------------------------------------------
void __fastcall TMainForm::bDeleteStudentClick(TObject * Sender) {
	if (sMainGrid->Selection.Top == sMainGrid->Selection.Bottom) {
		if (sMainGrid->Row == sMainGrid->RowCount - 1 || sMainGrid->Row == 0 ||
			bDeleteStudent->Tag == 1) {
			bDeleteStudent->Tag = 0;
			Application->MessageBox(String(L" Невозможно удалить данную строку")
				.w_str(), Application->Title.w_str(), MB_OK | MB_ICONERROR);
			return;
		}
		else
			switch (Application->MessageBox((String(L"Вы действительно ") +
				String(L"хотите удалить студента \"") +
				sMainGrid->Cells[0][sMainGrid->Row] + String(L"\"?")).w_str(),
				Application->Title.w_str(), MB_YESNO | MB_ICONQUESTION)) {
			case mrNo:
				return;
			case mrYes: ;
			}

		((TAdditionalSGrid*)sMainGrid)->DeleteRow(sMainGrid->Row);
	}
	else if (sMainGrid->Selection.Top > 0 && sMainGrid->Selection.Bottom <
		sMainGrid->RowCount - 1) {
		switch (Application->MessageBox((String(L"Вы действительно ") +
			String(L"хотите удалить студентов c \"") +
			sMainGrid->Cells[0][sMainGrid->Selection.Top] + String(L"\" по \"")
			+ sMainGrid->Cells[0][sMainGrid->Selection.Bottom] + String(L"\"?"))
			.w_str(), Application->Title.w_str(), MB_YESNO | MB_ICONQUESTION)) {
		case mrNo:
			return;
		case mrYes: ;
		}

		int selectionBottom = sMainGrid->Selection.Bottom, selectionTop =
			sMainGrid->Selection.Top;

		deleteSorted();

		for (int i = selectionBottom; i >= selectionTop; i--)
			if (sMainGrid->RowHeights[i] != -1)
				((TAdditionalSGrid*)sMainGrid)->DeleteRow(i);
	}
	else {
		Application->MessageBox(String(L"Выбрана нередактируемая строка").w_str
			(), Application->Title.w_str(), MB_OK | MB_ICONERROR);
		return;
	}

	if (MainForm->Caption.SubString(MainForm->Caption.Length(), 1) != L"*")
		MainForm->Caption = MainForm->Caption + L"*";
	menuSave->Enabled = true;

	DetermineAvgMarks();
	if (menuAutoWidthCol->Checked)
		AutoSizeGrid(sMainGrid);
}

// ---------------------------------------------------------------------------
void __fastcall TMainForm::bDeleteSubjectClick(TObject * Sender) {
	if (sMainGrid->Selection.Left == sMainGrid->Selection.Right) {
		if (sMainGrid->Col == sMainGrid->ColCount - 1 || sMainGrid->Col == 0 ||
			bDeleteSubject->Tag == 1) {
			bDeleteSubject->Tag = 0;
			Application->MessageBox(String(L"Невозможно удалить данный столбец")
				.w_str(), Application->Title.w_str(), MB_OK | MB_ICONERROR);
			return;
		}
		else
			switch (Application->MessageBox((String(L"Вы действительно ") +
				String(L"хотите удалить предмет \"") +
				sMainGrid->Cells[sMainGrid->Col][0] + String(L"\"?")).w_str(),
				Application->Title.w_str(), MB_YESNO | MB_ICONQUESTION)) {
			case mrNo:
				return;
			case mrYes: ;
			}

		((TAdditionalSGrid*)sMainGrid)->DeleteColumn(sMainGrid->Col);
	}
	else if (sMainGrid->Selection.Left > 0 && sMainGrid->Selection.Right <
		sMainGrid->ColCount - 1) {
		switch (Application->MessageBox((String(L"Вы действительно ") +
			String(L"хотите удалить предметы c \"") +
			sMainGrid->Cells[sMainGrid->Selection.Left][0] + String(L"\" по \"")
			+ sMainGrid->Cells[sMainGrid->Selection.Right][0] + String(L"\"?"))
			.w_str(), Application->Title.w_str(), MB_YESNO | MB_ICONQUESTION)) {
		case mrNo:
			return;
		case mrYes: ;
		}

		int selectionRight = sMainGrid->Selection.Right, selectionLeft =
			sMainGrid->Selection.Left;

		deleteSorted();

		for (int i = selectionRight; i >= selectionLeft; i--)
			((TAdditionalSGrid*)sMainGrid)->DeleteColumn(i);
	}
	else {
		Application->MessageBox(String(L"Выбран нередактируемый столбец").w_str
			(), Application->Title.w_str(), MB_OK | MB_ICONERROR);
		return;
	}

	if (MainForm->Caption.SubString(MainForm->Caption.Length(), 1) != L"*")
		MainForm->Caption = MainForm->Caption + L"*";
	menuSave->Enabled = true;

	DetermineAvgMarks();
	if (menuAutoWidthCol->Checked)
		AutoSizeGrid(sMainGrid);
}

// ---------------------------------------------------------------------------
void __fastcall TMainForm::menuAddStudentClick(TObject * Sender) {
	bAddStudentClick(Sender);
}
// ---------------------------------------------------------------------------

void __fastcall TMainForm::menuInsertStudentClick(TObject * Sender) {
	bInsertStudentClick(Sender);
}
// ---------------------------------------------------------------------------

void __fastcall TMainForm::menuDeleteStudentClick(TObject * Sender) {
	bDeleteStudentClick(Sender);
}
// ---------------------------------------------------------------------------

void __fastcall TMainForm::menuAddSubjectClick(TObject * Sender) {
	bAddSubjectClick(Sender);
}
// ---------------------------------------------------------------------------

void __fastcall TMainForm::menuInsertSubjectClick(TObject * Sender) {
	bInsertSubjectClick(Sender);
}
// ---------------------------------------------------------------------------

void __fastcall TMainForm::menuDeleteSubjectClick(TObject * Sender) {
	bDeleteSubjectClick(Sender);
}
// ---------------------------------------------------------------------------

void __fastcall TMainForm::menuAutoWidthColClick(TObject * Sender) {
	menuAutoWidthCol->Checked = menuAutoWidthCol->Checked ? false : true;
	if (menuAutoWidthCol->Checked)
		AutoSizeGrid(sMainGrid);
}
// ---------------------------------------------------------------------------

void __fastcall TMainForm::menuClearAllClick(TObject * Sender)
	// Очистить всё
{
	if (sMainGrid->RowHeights[1] == -1 && sMainGrid->ColWidths[1] == -1)
		return;

	switch (Application->MessageBox(String(L"Очистить все данные?").w_str(),
		Application->Title.w_str(), MB_YESNO | MB_ICONQUESTION)) {
	case mrYes:
		break;
	case mrNo:
		return;
	}

	for (int i = 0; i < sMainGrid->RowCount; i++)
		sMainGrid->Rows[i]->Clear();

	sMainGrid->Cells[0][0] = L"Фамилия \\ Предмет";

	DetermineAvgMarks();
	if (menuAutoWidthCol->Checked)
		AutoSizeGrid(sMainGrid);

	if (MainForm->Caption.SubString(MainForm->Caption.Length(), 1) != L"*")
		MainForm->Caption = MainForm->Caption + L"*";
	menuSave->Enabled = true;
}
// ---------------------------------------------------------------------------

void __fastcall TMainForm::menuFontClick(TObject * Sender) // Шрифт...
{
	FontDlg->Tag = 0;
	FontDlg->Font->Assign(sMainGrid->Font);
	TFont* srcFont = new TFont();

	try {
		srcFont->Assign(sMainGrid->Font);
		if (!FontDlg->Execute()) {
			if (FontDlg->Tag) {
				sMainGrid->Font->Assign(srcFont);

				if (menuAutoWidthCol->Checked)
					AutoSizeGrid(sMainGrid);
			}
			return;
		}
	}
	__finally {
		delete srcFont;
	}

	sMainGrid->Font->Assign(FontDlg->Font);

	if (menuAutoWidthCol->Checked)
		AutoSizeGrid(sMainGrid);
}
// ---------------------------------------------------------------------------

void __fastcall TMainForm::FontDlgApply(TObject * Sender, HWND Wnd) {
	FontDlg->Tag = 1;
	sMainGrid->Font->Assign(FontDlg->Font);
	if (menuAutoWidthCol->Checked)
		AutoSizeGrid(sMainGrid);
}

// ---------------------------------------------------------------------------
void __fastcall TMainForm::sMainGridGetEditText(TObject * Sender, int ACol,
	int ARow, UnicodeString & Value) {
	// Запрет редактирования ячеек средних оценок
	if (sMainGrid->Tag) {
		sMainGrid->Tag = false;
		return;
	}

	tempCell = sMainGrid->Cells[ACol][ARow];

	if (sMainGrid->Col == sMainGrid->ColCount - 1 ||
		sMainGrid->Row == sMainGrid->RowCount - 1) {
		StatusBar->SimpleText = L"❌ Редактирование данной ячейки запрещено";

		panelWithGrid->SetFocus();
		Beep();
		sMainGrid->Cells[ACol][ARow] = tempCell;
		return;
	}

	if (menuAutoWidthCol->Checked)
		AutoSizeGrid(sMainGrid);
}
// ---------------------------------------------------------------------------

void __fastcall TMainForm::sMainGridSelectCell(TObject * Sender, int ACol,
	int ARow, bool &CanSelect) {
	StatusBar->SimpleText = L"✅ Готово к работе";

	if (sMainGrid->RowHeights[ARow] == -1 || sMainGrid->ColWidths[ACol] == -1) {
		CanSelect = false;
		return;
	}

	sMainGrid->Options >> goEditing;

	if (ARow != 0)
		sMainGrid->FixedRows = 1;
	if (ACol != 0)
		sMainGrid->FixedCols = 1;

	if (!sMainGrid->EditorMode)
		return;

	// Фамилии
	{
		String tempStr = sMainGrid->Cells[sMainGrid->Col][sMainGrid->Row];
		if (sMainGrid->Col == 0 && sMainGrid->Row > 0 && sMainGrid->Row <
			sMainGrid->RowCount - 1 && tempStr.Length() != 0) {
			for (int i = 1; i < tempStr.Length(); i++) {
				if (!(tempStr[i] >= L'а' && tempStr[i] <= L'я') && !
					(tempStr[i] >= L'А' && tempStr[i] <= L'Я')
					&& tempStr[i] != L' ') {
					sMainGrid->Tag = true;

					StatusBar->SimpleText = L"❌ Некорректный ввод в ячейке [" +
						IntToStr(sMainGrid->Col) + L"][" +
						IntToStr(sMainGrid->Row) + L"]";

					Application->MessageBox
						((L"Значение \"" + sMainGrid->Cells[sMainGrid->Col]
						[sMainGrid->Row] + L"\"" + L" не явлется корректным")
						.w_str(), Application->Title.w_str(),
						MB_OK | MB_ICONERROR);

					sMainGrid->Cells[sMainGrid->Col][sMainGrid->Row] = tempCell;
					CanSelect = false;

					sMainGrid->EditorMode = true;
					return;
				}
			}
		}
	}

	// Предметы
	{
		String tempStr = sMainGrid->Cells[sMainGrid->Col][sMainGrid->Row];
		if (sMainGrid->Row == 0 && sMainGrid->Col > 0 && sMainGrid->Col <
			sMainGrid->ColCount - 1 && tempStr.Length() != 0) {
			for (int i = 1; i < tempStr.Length(); i++) {
				if (!(tempStr[i] >= L'а' && tempStr[i] <= L'я') && !
					(tempStr[i] >= L'А' && tempStr[i] <= L'Я')
					&& tempStr[i] != L' ' && !(tempStr[i] >=
					L'0' && tempStr[i] <= L'9')) {
					sMainGrid->Tag = true;

					StatusBar->SimpleText = L"❌ Некорректный ввод в ячейке [" +
						IntToStr(sMainGrid->Col) + L"][" +
						IntToStr(sMainGrid->Row) + L"]";

					Application->MessageBox
						((L"Значение \"" + sMainGrid->Cells[sMainGrid->Col]
						[sMainGrid->Row] + L"\"" + L" не явлется корректным")
						.w_str(), Application->Title.w_str(),
						MB_OK | MB_ICONERROR);

					sMainGrid->Cells[sMainGrid->Col][sMainGrid->Row] = tempCell;
					CanSelect = false;

					sMainGrid->EditorMode = true;
					return;
				}
			}
		}
	}

	// Оценки
	{
		int tInt;
		if (sMainGrid->Col > 0 && sMainGrid->Row >
			0 && sMainGrid->Col != sMainGrid->ColCount -
			1 && sMainGrid->Row != sMainGrid->RowCount - 1) {
			if (!TryStrToInt(sMainGrid->Cells[sMainGrid->Col][sMainGrid->Row],
				tInt) && sMainGrid->Cells[sMainGrid->Col][sMainGrid->Row]
				!= L"") {
				sMainGrid->Tag = true;
				StatusBar->SimpleText = L"❌ Некорректный ввод в ячейке [" +
					IntToStr(sMainGrid->Col) + L"][" +
					IntToStr(sMainGrid->Row) + L"]";

				Application->MessageBox
					((L"Значение \"" + sMainGrid->Cells[sMainGrid->Col]
					[sMainGrid->Row] + L"\"" +
					L" не явлется корректной оценкой.").w_str(),
					Application->Title.w_str(), MB_OK | MB_ICONERROR);

				sMainGrid->Cells[sMainGrid->Col][sMainGrid->Row] = tempCell;
				CanSelect = false;

				sMainGrid->EditorMode = true;
				return;
			}
		}
	} DetermineAvgMarks();

	if (menuAutoWidthCol->Checked)
		AutoSizeGrid(sMainGrid);
	if (tempCell != sMainGrid->Cells[sMainGrid->Col][sMainGrid->Row]) {
		if (MainForm->Caption.SubString(MainForm->Caption.Length(), 1) != L"*")
			MainForm->Caption = MainForm->Caption + L"*";
		menuSave->Enabled = true;
	}

}
// ---------------------------------------------------------------------------

void __fastcall TMainForm::sMainGridDblClick(TObject * Sender) {
	sMainGrid->Options << goEditing;

	TPoint curPos;
	GetCursorPos(&curPos); // curPos=Mouse->CursorPos;
	curPos = sMainGrid->ScreenToClient(curPos);

	if (KeyboardStateToShiftState() != TShiftState()
		<< ssCtrl << ssLeft && sMainGrid->FixedCols != 0 &&
		sMainGrid->FixedRows != 0) {
		if (GetCursor() == Screen->Cursors[crHSplit]) {
			TGridCoord gc = sMainGrid->MouseCoord(curPos.X - 4, curPos.Y);

			int maxColWidth = GetMaxColWidth(sMainGrid, gc.X);
			if (maxColWidth > sMainGrid->DefaultColWidth)
				sMainGrid->ColWidths[gc.X] = maxColWidth + 12;
			else
				sMainGrid->ColWidths[gc.X] = sMainGrid->DefaultColWidth + 12;
		}
		else {
			TGridCoord gc = sMainGrid->MouseCoord(curPos.X, curPos.Y);
			// Нажатие на нулевую ячейку
			if (gc.X == 0 && gc.Y == 0) {

				if (curPos.X < sMainGrid->ColWidths[0] / 2) {
					int i = abs(FSortedColNbr) - 1;

					{
						int k = abs(FSortedRowNbr) - 1;
						if (FSortedRowNbr)
							sMainGrid->Cells[0][k] =
								sMainGrid->Cells[0][k].SubString(1,
							sMainGrid->Cells[0][k].Length() - 2);
						FSortedRowNbr = 0;
					}

					if (FSortedColNbr)
						sMainGrid->Cells[i][0] =
							sMainGrid->Cells[i][0].SubString(1,
						sMainGrid->Cells[i][0].Length() - 2);

					if (FSortedColNbr && i == gc.X)
						FSortedColNbr = -FSortedColNbr;
					else
						FSortedColNbr = gc.X + 1;
					i = gc.X;

					sMainGrid->Cells[i][0] = FSortedColNbr < 0 ?
						sMainGrid->Cells[i][0] + L" \x25bc" :
						sMainGrid->Cells[i][0] + L" \x25b2";

					sMainGrid->Rows[sMainGrid->Row]->Objects[0] =
						(TObject*)true;

					SortGridByCol(sMainGrid, 0, FSortedColNbr > 0, cmpFuncText);

					for (i = 1; i < sMainGrid->RowCount - 1; i++)
						if (sMainGrid->Rows[i]->Objects[0]) {
							sMainGrid->Rows[i]->Objects[0] = (TObject*)false;
							sMainGrid->Row = i;
							break;
						}
				}
				else if (curPos.X > sMainGrid->ColWidths[0] / 2) {
					int i = abs(FSortedRowNbr) - 1;

					{
						int k = abs(FSortedColNbr) - 1;
						if (FSortedColNbr)
							sMainGrid->Cells[k][0] =
								sMainGrid->Cells[k][0].SubString(1,
							sMainGrid->Cells[k][0].Length() - 2);
						FSortedColNbr = 0;
					}

					if (FSortedRowNbr)
						sMainGrid->Cells[0][i] =
							sMainGrid->Cells[0][i].SubString(1,
						sMainGrid->Cells[0][i].Length() - 2);

					if (FSortedRowNbr && i == gc.Y)
						FSortedRowNbr = -FSortedRowNbr;
					else
						FSortedRowNbr = gc.Y + 1;
					i = gc.Y;

					sMainGrid->Cells[0][i] = FSortedRowNbr < 0 ?
						sMainGrid->Cells[0][i] + L" ◄" :
						sMainGrid->Cells[0][i] + L" ►";

					sMainGrid->Cols[sMainGrid->Col]->Objects[0] =
						(TObject*)true;

					SortGridByRow(sMainGrid, 0, FSortedRowNbr > 0, cmpFuncText);

					for (i = 1; i < sMainGrid->ColCount - 1; i++)
						if (sMainGrid->Cols[i]->Objects[0]) {
							sMainGrid->Cols[i]->Objects[0] = (TObject*)false;
							sMainGrid->Col = i;
							break;
						}
				}

			}
			else if (gc.Y == 0) { // Заголовок столбца?
				int i = abs(FSortedColNbr) - 1;

				{
					int k = abs(FSortedRowNbr) - 1;
					if (FSortedRowNbr)
						sMainGrid->Cells[0][k] =
							sMainGrid->Cells[0][k].SubString(1,
						sMainGrid->Cells[0][k].Length() - 2);
					FSortedRowNbr = 0;
				}

				if (FSortedColNbr)
					sMainGrid->Cells[i][0] =
						sMainGrid->Cells[i][0].SubString(1,
					sMainGrid->Cells[i][0].Length() - 2);

				if (FSortedColNbr && i == gc.X)
					FSortedColNbr = -FSortedColNbr;
				else
					FSortedColNbr = gc.X + 1;
				i = gc.X;

				sMainGrid->Cells[i][0] = FSortedColNbr < 0 ?
					sMainGrid->Cells[i][0] + L" \x25bc" :
					sMainGrid->Cells[i][0] + L" \x25b2";

				// Для сохранения выделения (1)
				sMainGrid->Rows[sMainGrid->Row]->Objects[0] = (TObject*)true;

				if (i == 0)
					SortGridByCol(sMainGrid, i, FSortedColNbr > 0, cmpFuncText);
				else if (i == sMainGrid->ColCount - 1)
					SortGridByCol(sMainGrid, i, FSortedColNbr > 0,
					cmpFuncFloatNumbers);
				else
					SortGridByCol(sMainGrid, i, FSortedColNbr > 0,
					cmpFuncIntNumbers);

				// Для сохранения выделения (1)
				for (i = 1; i < sMainGrid->RowCount - 1; i++)
					if (sMainGrid->Rows[i]->Objects[0]) {
						sMainGrid->Rows[i]->Objects[0] = (TObject*)false;
						sMainGrid->Row = i;
						break;
					}
			}
			else if (gc.X == 0) { // Заголовок строки?
				int i = abs(FSortedRowNbr) - 1;

				{
					int k = abs(FSortedColNbr) - 1;
					if (FSortedColNbr)
						sMainGrid->Cells[k][0] =
							sMainGrid->Cells[k][0].SubString(1,
						sMainGrid->Cells[k][0].Length() - 2);
					FSortedColNbr = 0;
				}

				if (FSortedRowNbr)
					sMainGrid->Cells[0][i] =
						sMainGrid->Cells[0][i].SubString(1,
					sMainGrid->Cells[0][i].Length() - 2);

				if (FSortedRowNbr && i == gc.Y)
					FSortedRowNbr = -FSortedRowNbr;
				else
					FSortedRowNbr = gc.Y + 1;
				i = gc.Y;

				sMainGrid->Cells[0][i] = FSortedRowNbr < 0 ?
					sMainGrid->Cells[0][i] + L" ◄" :
					sMainGrid->Cells[0][i] + L" ►";

				// Для сохранения выделения (1)
				sMainGrid->Cols[sMainGrid->Col]->Objects[0] = (TObject*)true;

				if (i == 0)
					SortGridByRow(sMainGrid, i, FSortedRowNbr > 0, cmpFuncText);
				else if (i == sMainGrid->RowCount - 1)
					SortGridByRow(sMainGrid, i, FSortedRowNbr > 0,
					cmpFuncFloatNumbers);
				else
					SortGridByRow(sMainGrid, i, FSortedRowNbr > 0,
					cmpFuncIntNumbers);

				// Для сохранения выделения (1)
				for (i = 1; i < sMainGrid->ColCount - 1; i++)
					if (sMainGrid->Cols[i]->Objects[0]) {
						sMainGrid->Cols[i]->Objects[0] = (TObject*)false;
						sMainGrid->Col = i;
						break;
					}
			}

			if (gc.Y != 0 && gc.X != 0) {
				if (cmbBox->ItemIndex != 0) {
					Application->MessageBox
						(String(
						L"Выберите фильтр \"Все студенты\" для редактирования информации")
						.w_str(), Application->Title.w_str(),
						MB_OK | MB_ICONWARNING);
					return;
				}
				sMainGrid->EditorMode = true;
				sMainGrid->SetFocus();
			}
			if (menuAutoWidthCol->Checked)
				AutoSizeGrid(sMainGrid);

			return;
		}
	}
	else {

		if (cmbBox->ItemIndex != 0) {
			Application->MessageBox
				(String(
				L"Выберите фильтр \"Все студенты\" для редактирования информации")
				.w_str(), Application->Title.w_str(), MB_OK | MB_ICONWARNING);
			return;
		}

		deleteSorted();

		{
			TGridCoord gc = sMainGrid->MouseCoord(curPos.X, curPos.Y);
			if (gc.Y == 0 && gc.X >=
				sMainGrid->FixedCols && gc.X != sMainGrid->ColCount - 1) {
				sMainGrid->FixedRows = 0;
				sMainGrid->Row = 0;
				sMainGrid->Col = gc.X;
				sMainGrid->EditorMode = true;
				sMainGrid->SetFocus();
			}
			else if (gc.X == 0 && gc.Y >=
				sMainGrid->FixedRows && gc.Y != sMainGrid->RowCount - 1) {
				sMainGrid->FixedCols = 0;
				sMainGrid->Col = 0;
				sMainGrid->Row = gc.Y;
				sMainGrid->EditorMode = true;
				sMainGrid->SetFocus();

			}
			else if ((gc.Y == 0 && gc.X == sMainGrid->ColCount - 1) ||
				(gc.X == 0 && gc.Y == sMainGrid->RowCount - 1)) {
				StatusBar->SimpleText =
					L"❌ Редактирование данной ячейки запрещено";
				sMainGrid->Tag = true;
				Beep();
			}
		}
	}
}

// ---------------------------------------------------------------------------
void __fastcall TMainForm::sMainGridKeyPress(TObject * Sender,
	System::WideChar & Key) {

	if (cmbBox->ItemIndex != 0) {
		Application->MessageBox
			(String(
			L"Выберите фильтр \"Все студенты\" для редактирования информации")
			.w_str(), Application->Title.w_str(), MB_OK | MB_ICONWARNING);
		return;
	}

	StatusBar->SimpleText = L"✅ Готово к работе";

	sMainGrid->Options << goEditing;

	if (Key == VK_ESCAPE && sMainGrid->EditorMode) {
		sMainGrid->Cells[sMainGrid->Col][sMainGrid->Row] = tempCell;
		sMainGrid->EditorMode = false;
	}

	// Фамилии
	if (sMainGrid->Col == 0 && sMainGrid->Row > 0 && sMainGrid->Row <
		sMainGrid->RowCount - 1) {
		if (!(Key >= L'а' && Key <= L'я') && !(Key >= L'А' && Key <= L'Я')
			&& Key != L'-' && Key != L' ' && Key != VK_RETURN && Key !=
			VK_BACK && Key != VK_ESCAPE && Key != 22 && Key != 26 && Key !=
			24 && Key != 3 && Key != VK_TAB) {
			StatusBar->SimpleText = L"❌ Недопустимый символ '" +
				UnicodeString(Key) + L"\' в ячейке [" +
				IntToStr(sMainGrid->Col + 1) + L"][" +
				IntToStr(sMainGrid->Row + 1) + L"]. Возможные символы а-я," +
				L" А-Я и пробел";
			Key = 0;
			Beep();
			return;
		}
		sMainGrid->EditorMode = true;
	}
	// Предметы
	if (sMainGrid->Row == 0 && sMainGrid->Col > 0 && sMainGrid->Col <
		sMainGrid->ColCount - 1) {
		if (!(Key >= L'а' && Key <= L'я') && !(Key >= L'А' && Key <= L'Я')
			&& Key != L'-' && Key != L' ' && !(Key >= L'0' && Key <= L'9')
			&& Key != VK_RETURN && Key != VK_BACK && Key != VK_ESCAPE && Key !=
			22 && Key != 26 && Key != 24 && Key != 3 && Key != VK_TAB) {
			StatusBar->SimpleText = L"❌ Недопустимый символ '" +
				UnicodeString(Key) + L"\' в ячейке [" +
				IntToStr(sMainGrid->Col + 1) + L"][" +
				IntToStr(sMainGrid->Row + 1) + L"]";
			Key = 0;
			Beep();
			return;
		}
	}
	// Оценки
	if (sMainGrid->Col > 0 && sMainGrid->Row >
		0 && sMainGrid->Row != sMainGrid->RowCount -
		1 && sMainGrid->Col != sMainGrid->ColCount - 1) {
		if (!(Key >= L'2' && Key <= L'5')
			&& Key != VK_RETURN && Key != VK_BACK && Key != VK_ESCAPE && Key !=
			22 && Key != 26 && Key != 24 && Key != 3 && Key != VK_TAB) {
			StatusBar->SimpleText = L"❌ Недопустимый символ '" +
				UnicodeString(Key) + L"\' в ячейке [" +
				IntToStr(sMainGrid->Col + 1) + L"][" +
				IntToStr(sMainGrid->Row + 1) +
				"]. (Оценка должна быть от 2 до 5)";
			Key = 0;
			Beep();
		}
		else if (Key >= L'2' && Key <= L'5') {
			sMainGrid->Cells[sMainGrid->Col][sMainGrid->Row] = (AnsiString)Key;
			DetermineAvgMarks();
		}
		if (sMainGrid->Cells[sMainGrid->Col][sMainGrid->Row] == L"")
			DetermineAvgMarks();
	}

	if (menuAutoWidthCol->Checked)
		AutoSizeGrid(sMainGrid);
	sMainGrid->Repaint();

	if (tempCell != sMainGrid->Cells[sMainGrid->Col][sMainGrid->Row]) {
		if (MainForm->Caption.SubString(MainForm->Caption.Length(), 1) != L"*")
			MainForm->Caption = MainForm->Caption + L"*";
		menuSave->Enabled = true;
	}
}

// ---------------------------------------------------------------------------
void __fastcall TMainForm::sMainGridSetEditText(TObject * Sender, int ACol,
	int ARow, const UnicodeString Value) {
	bDeleteStudent->Tag = 0;
	bDeleteSubject->Tag = 0;

	// Фамилии
	{
		String tempStr = sMainGrid->Cells[sMainGrid->Col][sMainGrid->Row];
		if (sMainGrid->Col == 0 && sMainGrid->Row > 0 && sMainGrid->Row <
			sMainGrid->RowCount - 1 && tempStr.Length() != 0) {
			for (int i = 1; i < tempStr.Length(); i++) {
				if (!(tempStr[i] >= L'а' && tempStr[i] <= L'я') && !
					(tempStr[i] >= L'А' && tempStr[i] <= L'Я')
					&& tempStr[i] != L'-' && tempStr[i] != L' ') {
					sMainGrid->Tag = true;

					StatusBar->SimpleText = L"❌ Некорректный ввод в ячейке [" +
						IntToStr(sMainGrid->Col) + L"][" +
						IntToStr(sMainGrid->Row) + L"]";

					Application->MessageBox
						((L"Значение \"" + sMainGrid->Cells[sMainGrid->Col]
						[sMainGrid->Row] + L"\"" + L" не явлется корректным")
						.w_str(), Application->Title.w_str(),
						MB_OK | MB_ICONERROR);

					sMainGrid->Cells[sMainGrid->Col][sMainGrid->Row] = tempCell;

					sMainGrid->EditorMode = true;
					return;
				}
			}
		}
	}

	// Предметы
	{
		String tempStr = sMainGrid->Cells[sMainGrid->Col][sMainGrid->Row];
		if (sMainGrid->Row == 0 && sMainGrid->Col > 0 && sMainGrid->Col <
			sMainGrid->ColCount - 1 && tempStr.Length() != 0) {
			for (int i = 1; i < tempStr.Length(); i++) {
				if (!(tempStr[i] >= L'а' && tempStr[i] <= L'я') && !
					(tempStr[i] >= L'А' && tempStr[i] <= L'Я')
					&& tempStr[i] != L'-' && tempStr[i] != L' ' && !
					(tempStr[i] >= L'0' && tempStr[i] <= L'9')) {
					sMainGrid->Tag = true;

					StatusBar->SimpleText = L"❌ Некорректный ввод в ячейке [" +
						IntToStr(sMainGrid->Col) + L"][" +
						IntToStr(sMainGrid->Row) + L"]";

					Application->MessageBox
						((L"Значение \"" + sMainGrid->Cells[sMainGrid->Col]
						[sMainGrid->Row] + L"\"" + L" не явлется корректным")
						.w_str(), Application->Title.w_str(),
						MB_OK | MB_ICONERROR);

					sMainGrid->Cells[sMainGrid->Col][sMainGrid->Row] = tempCell;

					sMainGrid->EditorMode = true;
					return;
				}
			}
		}
	}

	// Оценки
	{
		int tInt;
		if (sMainGrid->Col > 0 && sMainGrid->Row >
			0 && sMainGrid->Col != sMainGrid->ColCount -
			1 && sMainGrid->Row != sMainGrid->RowCount - 1) {
			if (!TryStrToInt(sMainGrid->Cells[sMainGrid->Col][sMainGrid->Row],
				tInt) && sMainGrid->Cells[sMainGrid->Col][sMainGrid->Row]
				!= L"") {
				sMainGrid->Tag = true;
				StatusBar->SimpleText = L"❌ Некорректный ввод в ячейке [" +
					IntToStr(sMainGrid->Col) + L"][" +
					IntToStr(sMainGrid->Row) + L"]";

				Application->MessageBox
					((L"Значение \"" + sMainGrid->Cells[sMainGrid->Col]
					[sMainGrid->Row] + L"\"" +
					L" не явлется корректной оценкой.").w_str(),
					Application->Title.w_str(), MB_OK | MB_ICONERROR);

				sMainGrid->Cells[sMainGrid->Col][sMainGrid->Row] = tempCell;

				sMainGrid->EditorMode = true;
				return;
			}
		}
	}

	if (ARow == 0 && !sMainGrid->EditorMode) {
		sMainGrid->FixedRows = 1;
		sMainGrid->Col = ACol;
		bDeleteStudent->Tag = 1; // Для запрета удаления нулевой строки
	}
	else if (ACol == 0 && !sMainGrid->EditorMode) {
		sMainGrid->FixedCols = 1;
		sMainGrid->Row = ARow;
		bDeleteSubject->Tag = 1; // Для запрета удаления нулевого столбца
	}

	DetermineAvgMarks();
	if (menuAutoWidthCol->Checked)
		AutoSizeGrid(sMainGrid);

	if (tempCell != sMainGrid->Cells[sMainGrid->Col][sMainGrid->Row]) {
		if (MainForm->Caption.SubString(MainForm->Caption.Length(), 1) != L"*")
			MainForm->Caption = MainForm->Caption + L"*";
		menuSave->Enabled = true;
	}
}

// ---------------------------------------------------------------------------
void __fastcall TMainForm::cmbBoxChange(TObject * Sender) {
	switch (cmbBox->ItemIndex) {
	case 0: { // Все студенты
			if (sMainGrid->RowCount == 2 && sMainGrid->RowHeights[1] == -1)
				return;
			for (int i = 1; i < sMainGrid->RowCount - 1; i++)
				sMainGrid->RowHeights[i] = sMainGrid->DefaultRowHeight;
			sMainGrid->Options << goRowSizing << goColSizing;
			break;
		}
	case 1: { // Имеющие неудовлетворительные оценки
			if (sMainGrid->RowCount == 2 && sMainGrid->RowHeights[1] == -1)
				return;
			for (int i = 1; i < sMainGrid->RowCount - 1; i++)
				sMainGrid->RowHeights[i] = sMainGrid->DefaultRowHeight;

			bool isHave2;
			for (int x = 1; x < sMainGrid->RowCount - 1; x++) {
				isHave2 = false;
				for (int y = 1; y < sMainGrid->ColCount - 1; y++)
					if (sMainGrid->Cells[y][x] == L"2")
						isHave2 = true;
				if (!isHave2) {
					sMainGrid->RowHeights[x] = -1;
					if (sMainGrid->Row == x && sMainGrid->Row !=
						sMainGrid->RowCount - 1)
						sMainGrid->Row++;
				}
				else
					sMainGrid->RowHeights[x] = sMainGrid->DefaultRowHeight;
			}
			sMainGrid->Options >> goRowSizing;
			break;
		}
	case 2: { // Только неаттестованные
			if (sMainGrid->RowCount == 2 && sMainGrid->RowHeights[1] == -1)
				return;
			for (int i = 1; i < sMainGrid->RowCount - 1; i++)
				sMainGrid->RowHeights[i] = sMainGrid->DefaultRowHeight;

			bool isNotHave;
			for (int x = 1; x < sMainGrid->RowCount - 1; x++) {
				isNotHave = false;
				for (int y = 1; y < sMainGrid->ColCount - 1; y++)
					if (sMainGrid->Cells[y][x] == L"")
						isNotHave = true;
				if (!isNotHave) {
					sMainGrid->RowHeights[x] = -1;
					if (sMainGrid->Row == x && sMainGrid->Row !=
						sMainGrid->RowCount - 1)
						sMainGrid->Row++;
				}
				else
					sMainGrid->RowHeights[x] = sMainGrid->DefaultRowHeight;
			}
			sMainGrid->Options >> goRowSizing;
			break;
		}
	case 3: {
			if (sMainGrid->RowCount == 2 && sMainGrid->RowHeights[1] == -1)
				return;
			for (int i = 1; i < sMainGrid->RowCount - 1; i++)
				sMainGrid->RowHeights[i] = sMainGrid->DefaultRowHeight;

			bool isAllMarks5;
			for (int x = 1; x < sMainGrid->RowCount - 1; x++) {
				isAllMarks5 = true;
				for (int y = 1; y < sMainGrid->ColCount - 1; y++)
					if (sMainGrid->Cells[y][x] != L"5")
						isAllMarks5 = false;
				if (!isAllMarks5) {
					sMainGrid->RowHeights[x] = -1;
					if (sMainGrid->Row == x && sMainGrid->Row !=
						sMainGrid->RowCount - 1)
						sMainGrid->Row++;
				}
				else
					sMainGrid->RowHeights[x] = sMainGrid->DefaultRowHeight;
			}
			sMainGrid->Options >> goRowSizing;
			break;
		}
	}

	sMainGrid->Repaint();

	while (sMainGrid->RowHeights[sMainGrid->Row] == -1)
		sMainGrid->Row++;

	DetermineAvgMarks();
}

// ---------------------------------------------------------------------------
void __fastcall TMainForm::sMainGridDrawCell(TObject * Sender, int ACol,
	int ARow, TRect & Rect, TGridDrawState State) {

	if (ARow == 0 && ACol > 0 && ACol < sMainGrid->ColCount - 1) {
		if (chLightSubjects->Checked)
			if (isColMaxMarksFive(sMainGrid, ACol, ARow)) {
				TRect r = Rect;
				sMainGrid->Canvas->Brush->Color = clWebPaleGreen;
				sMainGrid->Canvas->FillRect(r);

				sMainGrid->Canvas->TextOut
					(Rect.Left + (Rect.Width() - sMainGrid->Canvas->TextWidth
					(sMainGrid->Cells[ACol][ARow])) / 2,
					Rect.Top + (Rect.Height() - sMainGrid->Canvas->TextHeight
					(sMainGrid->Cells[ACol][ARow])) / 2,
					sMainGrid->Cells[ACol][ARow]);

			}
			else if (isColMaxMarksTwo(sMainGrid, ACol, ARow)) {
				TRect r = Rect;
				sMainGrid->Canvas->Brush->Color = clWebIndianRed;
				sMainGrid->Canvas->FillRect(r);

				sMainGrid->Canvas->TextOut
					(Rect.Left + (Rect.Width() - sMainGrid->Canvas->TextWidth
					(sMainGrid->Cells[ACol][ARow])) / 2,
					Rect.Top + (Rect.Height() - sMainGrid->Canvas->TextHeight
					(sMainGrid->Cells[ACol][ARow])) / 2,
					sMainGrid->Cells[ACol][ARow]);
			}
	}

	if (sMainGrid->RowHeights[ARow] == -1)
		return;
	if (sMainGrid->ColCount > 1 && sMainGrid->RowCount >
		1 && ((ARow == sMainGrid->RowCount - 1 && ACol != 0) ||
		(ACol == sMainGrid->ColCount - 1 && ARow != 0))) {
		TRect r = Rect;
		sMainGrid->Canvas->Brush->Color = clWebLavender;
		sMainGrid->Canvas->FillRect(r);

		sMainGrid->Canvas->TextOut
			(Rect.Left + (Rect.Width() - sMainGrid->Canvas->TextWidth
			(sMainGrid->Cells[ACol][ARow])) / 2,
			Rect.Top + (Rect.Height() - sMainGrid->Canvas->TextHeight
			(sMainGrid->Cells[ACol][ARow])) / 2, sMainGrid->Cells[ACol][ARow]);
	}
}

// ---------------------------------------------------------------------------
void __fastcall TMainForm::menuSaveFileResultsClick(TObject * Sender) {
	AnsiString oldFileName = saveFileDlg->FileName;

	saveFileDlg->Filter = "Текстовые файлы (*.txt)|*.TXT|Все файлы (*.*)|*.*";
    saveFileDlg->DefaultExt = L"txt";
	saveFileDlg->FileName = AnsiString(L"");
	if (!saveFileDlg->Execute())
		return;

	fstream file;
	file.open(AnsiString(saveFileDlg->FileName).c_str(), ios::out);
	if (!file.is_open()) {
		Application->MessageBox
			((String(L"Не удалось создать файл \"") + saveFileDlg->FileName +
			String(L"\"!")).w_str(), Application->Title.w_str(),
			MB_OK | MB_ICONERROR);
		return;
	}

	DynamicArray<short int>colWidths;
	try {
		colWidths.Length = sMainGrid->ColCount;
	}
	catch (...) {
		Application->MessageBox((String(L"Ошибка выделения оперативной памяти"))
			.w_str(), Application->Title.w_str(), MB_OK | MB_ICONERROR);
		return;
	}

	for (int idxCol = 0; idxCol < sMainGrid->ColCount; idxCol++) {
		colWidths[idxCol] = 0;
		for (int idxRow = 0; idxRow < sMainGrid->RowCount; idxRow++)
			if (colWidths[idxCol] < sMainGrid->Cells[idxCol][idxRow].Length())
				colWidths[idxCol] =
					sMainGrid->Cells[idxCol][idxRow].Length() + 6;
	}

	for (int idxRow = 0; idxRow < sMainGrid->RowCount; idxRow++) {
		if (sMainGrid->RowHeights[idxRow] == -1)
			continue;
		for (int idxCol = 0; idxCol < sMainGrid->ColCount; idxCol++) {
			if (sMainGrid->Cells[idxCol][idxRow] == L"") {
				file << std::setw(colWidths[idxCol]) << AnsiString(" ").c_str();
			}
			else
				file << std::setw(colWidths[idxCol]) << AnsiString
					(sMainGrid->Cells[idxCol][idxRow]).c_str();
		}
		file << "\n";
	}

	saveFileDlg->FileName = oldFileName;
	colWidths.Length = 0;
	file.close();

	StatusBar->SimpleText = L"✅ Результаты успешно сохранены в файл";
}

// ---------------------------------------------------------------------------
void __fastcall TMainForm::menuAboutProgramClick(TObject * Sender) {
	TfAbout *Form;
	Form = new TfAbout(this);
	Form->ShowModal();
	Form->Free();
}
// ---------------------------------------------------------------------------

void __fastcall TMainForm::menuLightSubjectsClick(TObject * Sender) {
	if (menuLightSubjects->Checked)
		menuLightSubjects->Checked = false;
	else
		menuLightSubjects->Checked = true;

	chLightSubjects->Checked = menuLightSubjects->Checked;
	sMainGrid->Repaint();
}
// ---------------------------------------------------------------------------

void __fastcall TMainForm::chLightSubjectsClick(TObject * Sender) {
	menuLightSubjects->Checked = chLightSubjects->Checked;
	sMainGrid->Repaint();
}
// ---------------------------------------------------------------------------

void __fastcall TMainForm::menuExitClick(TObject * Sender) {
	if (menuSave->Enabled)
		switch (Application->MessageBox
			((String(L"Сохранить изменения в файле \"") +
			saveFileDlg->FileName + "\"?").w_str(), Application->Title.w_str(),
			MB_YESNOCANCEL | MB_ICONQUESTION)) {
		case mrYes: {
				SaveDataWithDlgOrNo();
			}
		case mrCancel:
		case mrNo: ;
		}
	Close();
}
// ---------------------------------------------------------------------------

void __fastcall TMainForm::menuCorrectWidthColClick(TObject * Sender) {
	AutoSizeGrid(sMainGrid);
}
// ---------------------------------------------------------------------------

void __fastcall TMainForm::FormCloseQuery(TObject * Sender, bool &CanClose) {
	if (menuSave->Enabled)
		switch (Application->MessageBox
			((String(L"Сохранить изменения в файле \"") +
			saveFileDlg->FileName + "\"?").w_str(), Application->Title.w_str(),
			MB_YESNOCANCEL | MB_ICONQUESTION)) {
		case mrYes: {
				CanClose = false;
				SaveDataWithDlgOrNo();
				if (!menuSave->Enabled) {
					CanClose = true;
					return;
				}
				else
					return;
			}
		case mrCancel: {
				CanClose = false;
				return;
			}
		case mrNo: {
				return;
			}
		}
}

// ---------------------------------------------------------------------------
void __fastcall TMainForm::sMainGridRowMoved(TObject * Sender, int FromIndex,
	int ToIndex) {

	if (panelWithGrid->Tag == 1) {
		panelWithGrid->Tag = 0;
		return;
	}

	if (FromIndex == sMainGrid->RowCount - 1) {
		panelWithGrid->Tag = 1;
		((TAdditionalSGrid*)sMainGrid)->MoveRow(ToIndex,
			sMainGrid->RowCount - 1);
		Application->MessageBox(String(L"Перенос данной строки запрещён").w_str
			(), Application->Title.w_str(), MB_OK | MB_ICONERROR);
	}

	if (ToIndex == sMainGrid->RowCount - 1) {
		panelWithGrid->Tag = 1;
		((TAdditionalSGrid*)sMainGrid)->MoveRow(sMainGrid->RowCount - 1,
			sMainGrid->RowCount - 2);
	}

	for (int i = 0; i < sMainGrid->RowCount - 1; i++) {
		if (sMainGrid->Cells[0][i].Pos(L'◄') > 0) {
			FSortedRowNbr = -(i + 1);
			break;
		}
		else if (sMainGrid->Cells[0][i].Pos(L'►') > 0) {
			FSortedRowNbr = i + 1;
			break;
		}
	}
}
// ---------------------------------------------------------------------------

void __fastcall TMainForm::sMainGridColumnMoved(TObject * Sender, int FromIndex,
	int ToIndex) {
	if (panelWithButtons->Tag == 1) {
		panelWithButtons->Tag = 0;
		return;
	}

	if (FromIndex == sMainGrid->ColCount - 1) {
		panelWithButtons->Tag = 1;
		((TAdditionalSGrid*)sMainGrid)->MoveColumn(ToIndex,
			sMainGrid->ColCount - 1);
		Application->MessageBox(String(L"Перенос данного столбца запрещён")
			.w_str(), Application->Title.w_str(), MB_OK | MB_ICONERROR);
	}

	if (ToIndex == sMainGrid->ColCount - 1) {
		panelWithButtons->Tag = 1;
		((TAdditionalSGrid*)sMainGrid)->MoveColumn(sMainGrid->ColCount - 1,
			sMainGrid->ColCount - 2);
	}

	for (int i = 0; i < sMainGrid->ColCount - 1; i++) {
		if (sMainGrid->Cells[i][0].Pos(L'\x25bc') > 0) {
			FSortedColNbr = -(i + 1);
			break;
		}
		else if (sMainGrid->Cells[i][0].Pos(L'\x25b2') > 0) {
			FSortedColNbr = i + 1;
			break;
		}
	}
}
// ---------------------------------------------------------------------------
