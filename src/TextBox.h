#pragma once

typedef struct TextBox TextBox;

TextBox *CreateTextBox(int xoffset, int yoffset, GFont font, GRect frame, GTextAlignment align, bool scroll);
void InitializeTextBox(Layer *layer, TextBox *textBox, char *initialText);
void RemoveTextBox(TextBox *textBox);
void FreeTextBox(TextBox *textBox);

void TextBoxSetText(TextBox *textBox, const char *text);
const char *TextBoxGetText(TextBox *textBox);
bool TextBoxInitialized(TextBox *textBox);
void ShowTextBox(TextBox *textBox);
void HideTextBox(TextBox *textBox);

void TextBox_ScrollUp(TextBox *textBox);
void TextBox_ScrollDown(TextBox *textBox);
