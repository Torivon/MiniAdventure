#pragma once

typedef struct TextBox TextBox;

TextBox *CreateTextBox(int resourceId, int xoffset, int yoffset, GFont font, GRect frame);
void InitializeTextBox(Window *window, TextBox *textBox, char *initialText);
void RemoveTextBox(TextBox *textBox);
void FreeTextBox(TextBox *textBox);

void TextBoxSetText(TextBox *textBox, const char *text);
const char *TextBoxGetText(TextBox *textBox);
bool TextBoxInitialized(TextBox *textBox);
void ShowTextBox(TextBox *textBox);
void HideTextBox(TextBox *textBox);

