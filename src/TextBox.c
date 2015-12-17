#include <pebble.h>
#include "Logging.h"
#include "TextBox.h"

typedef struct TextBox
{
	int backgroundResourceId;
	int xoffset;
	int yoffset;
	GFont font;
	GRect frame;
	BitmapLayer *backgroundLayer;
	GBitmap *backgroundImage;
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

	layer_remove_from_parent(bitmap_layer_get_layer(textBox->backgroundLayer));
}

TextBox *CreateTextBox(int resourceId, int xoffset, int yoffset, GFont font, GRect frame)
{
	TextBox *textBox = calloc(sizeof(TextBox), 1);
	textBox->backgroundResourceId = resourceId;
	textBox->font = font;
	textBox->frame = frame;
	textBox->xoffset = xoffset;
	textBox->yoffset = yoffset;
	return textBox;
}

void InitializeTextBox(Window *window, TextBox *textBox, char *initialText)
{
	Layer *window_layer = window_get_root_layer(window);

	if(!textBox->initialized)
	{
		textBox->backgroundImage = gbitmap_create_with_resource(textBox->backgroundResourceId);
		GRect backgroundFrame = gbitmap_get_bounds(textBox->backgroundImage);
		textBox->frame.size = backgroundFrame.size;
		textBox->backgroundLayer = bitmap_layer_create(textBox->frame);
		bitmap_layer_set_bitmap(textBox->backgroundLayer, textBox->backgroundImage);
		bitmap_layer_set_alignment(textBox->backgroundLayer, GAlignCenter);
		layer_add_child(window_layer, bitmap_layer_get_layer(textBox->backgroundLayer));
		Layer *backgroundLayer = bitmap_layer_get_layer(textBox->backgroundLayer);
		GRect newFrame = layer_get_bounds(backgroundLayer);
		newFrame.origin.x += textBox->xoffset;
		newFrame.origin.y += textBox->yoffset;
		newFrame.size.w -= 2 * textBox->xoffset;
		newFrame.size.h -= 2 * textBox->yoffset;
		textBox->textLayer = text_layer_create(newFrame);
		text_layer_set_text_color(textBox->textLayer, GColorWhite);
		text_layer_set_background_color(textBox->textLayer, GColorClear);
		text_layer_set_font(textBox->textLayer, textBox->font);
		text_layer_set_text_alignment(textBox->textLayer, GTextAlignmentCenter);
		layer_add_child(backgroundLayer, text_layer_get_layer(textBox->textLayer));
		text_layer_set_text(textBox->textLayer, initialText);
		textBox->initialized = true;
	}
	layer_add_child(window_layer, bitmap_layer_get_layer(textBox->backgroundLayer));
}

void FreeTextBox(TextBox *textBox)
{
	if(textBox->initialized)
	{
		bitmap_layer_destroy(textBox->backgroundLayer);
		gbitmap_destroy(textBox->backgroundImage);
		text_layer_destroy(textBox->textLayer);
	}
	
	free(textBox);
}

void ShowTextBox(TextBox *textBox)
{
	if(!TextBoxInitialized(textBox))
		return;
	
	layer_set_hidden(bitmap_layer_get_layer(textBox->backgroundLayer), false);
}

void HideTextBox(TextBox *textBox)
{
	if(!TextBoxInitialized(textBox))
		return;
	
	layer_set_hidden(bitmap_layer_get_layer(textBox->backgroundLayer), true);

}