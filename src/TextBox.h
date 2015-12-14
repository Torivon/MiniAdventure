#pragma once

typedef struct TextBox TextBox;

TextBox *CreateTextBox(int resourceId, int xoffset, int yoffset, GFont font, GRect frame);
void InitializeTextBox(Window *window, TextBox *textBox, char *initialText);
void RemoveTextBox(TextBox *textBox);
void FreeTextBox(TextBox *textBox);

void TextBoxSetText(TextBox *textBox, char *text);
bool TextBoxInitialized(TextBox *textBox);

