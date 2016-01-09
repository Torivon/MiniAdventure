#include <pebble.h>
#include "Logging.h"
#include "OptionsMenu.h"
#include "TextBox.h"
#include "Utils.h"

typedef struct TextBox
{
	int xoffset;
	int yoffset;
	GFont font;
	GRect frame;
	Layer *mainLayer;
	TextLayer *textLayer;
	bool initialized;
} TextBox;

bool TextBoxInitialized(TextBox *textBox)
{
	return textBox && textBox->initialized;
}

void TextBoxSetText(TextBox *textBox, const char *text)
{
	if(!textBox || !textBox->initialized)
		return;
	
	text_layer_set_text(textBox->textLayer, text);
}

const char *TextBoxGetText(TextBox *textBox)
{
	if(!textBox || !textBox->initialized)
		return "";
	
	return text_layer_get_text(textBox->textLayer);
}

void RemoveTextBox(TextBox *textBox)
{
	if(!textBox || !textBox->initialized)
		return;

	layer_remove_from_parent(textBox->mainLayer);
}

TextBox *CreateTextBox(int xoffset, int yoffset, GFont font, GRect frame)
{
	TextBox *textBox = calloc(sizeof(TextBox), 1);
	textBox->font = font;
	textBox->frame = frame;
	textBox->xoffset = xoffset;
	textBox->yoffset = yoffset;
	return textBox;
}

void TextBoxUpdateProc(struct Layer *layer, GContext *ctx)
{
	GRect bounds = layer_get_bounds(layer);
	DrawContentFrame(ctx, &bounds);
}

void InitializeTextBox(Window *window, TextBox *textBox, char *initialText)
{
	Layer *window_layer = window_get_root_layer(window);

	if(!textBox->initialized)
	{
		textBox->mainLayer = layer_create(textBox->frame);
		layer_add_child(window_layer, textBox->mainLayer);
		GRect newFrame = layer_get_bounds(textBox->mainLayer);
		newFrame.origin.x += textBox->xoffset;
		newFrame.origin.y += textBox->yoffset;
		newFrame.size.w -= 2 * textBox->xoffset;
		newFrame.size.h -= 2 * textBox->yoffset;
		textBox->textLayer = text_layer_create(newFrame);
		text_layer_set_text_color(textBox->textLayer, GColorWhite);
		text_layer_set_background_color(textBox->textLayer, GColorClear);
		text_layer_set_font(textBox->textLayer, textBox->font);
		text_layer_set_text_alignment(textBox->textLayer, GTextAlignmentCenter);
		layer_add_child(textBox->mainLayer, (Layer*)textBox->textLayer);
		text_layer_set_text(textBox->textLayer, initialText);
		textBox->initialized = true;

		layer_set_update_proc(textBox->mainLayer, TextBoxUpdateProc);
	}
	layer_add_child(window_layer, textBox->mainLayer);
}

void FreeTextBox(TextBox *textBox)
{
    if(!textBox)
        return;
    
	if(textBox->initialized)
	{
		layer_destroy(textBox->mainLayer);
		text_layer_destroy(textBox->textLayer);
	}
	
	free(textBox);
}

void ShowTextBox(TextBox *textBox)
{
	if(!TextBoxInitialized(textBox))
		return;
	
    ShowLayer(textBox->mainLayer);
}

void HideTextBox(TextBox *textBox)
{
	if(!TextBoxInitialized(textBox))
		return;

    HideLayer(textBox->mainLayer);
}