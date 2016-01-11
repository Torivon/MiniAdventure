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
    bool allowScroll;
	Layer *mainLayer;
	TextLayer *textLayer;
    ScrollLayer *scrollLayer;
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
	
    GRect bounds = {.origin = {0}, .size = {0}};

    if(textBox->scrollLayer)
    {
        bounds = layer_get_bounds((Layer*)textBox->scrollLayer);
        text_layer_set_size(textBox->textLayer, GSize(bounds.size.w, 2000));
    }
    
	text_layer_set_text(textBox->textLayer, text);
    if(textBox->scrollLayer)
    {
        GPoint offset = GPointZero;
        scroll_layer_set_content_offset(textBox->scrollLayer, offset, false);
        GSize max_size = text_layer_get_content_size(textBox->textLayer);
        text_layer_set_size(textBox->textLayer, max_size);
        scroll_layer_set_content_size(textBox->scrollLayer, GSize(bounds.size.w, max_size.h + 4));
    }
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
    textBox->allowScroll = false;
	return textBox;
}

TextBox *CreateScrollTextBox(int xoffset, int yoffset, GFont font, GRect frame)
{
    TextBox *textBox = calloc(sizeof(TextBox), 1);
    textBox->font = font;
    textBox->frame = frame;
    textBox->xoffset = xoffset;
    textBox->yoffset = yoffset;
    textBox->allowScroll = true;
    return textBox;
}

void TextBoxUpdateProc(struct Layer *layer, GContext *ctx)
{
	GRect bounds = layer_get_bounds(layer);
	DrawContentFrame(ctx, &bounds);
}

void InitializeTextBox(Layer *layer, TextBox *textBox, char *initialText)
{
	if(!textBox->initialized)
	{
		textBox->mainLayer = layer_create(textBox->frame);
		GRect newFrame = layer_get_bounds(textBox->mainLayer);
		newFrame.origin.x += textBox->xoffset;
		newFrame.origin.y += textBox->yoffset;
		newFrame.size.w -= 2 * textBox->xoffset;
		newFrame.size.h -= 2 * textBox->yoffset;
        if(textBox->allowScroll)
        {
            textBox->scrollLayer = scroll_layer_create(newFrame);
            layer_add_child(textBox->mainLayer, (Layer*)textBox->scrollLayer);
        }
		textBox->textLayer = text_layer_create(newFrame);
		text_layer_set_text_color(textBox->textLayer, GColorWhite);
		text_layer_set_background_color(textBox->textLayer, GColorClear);
		text_layer_set_font(textBox->textLayer, textBox->font);
		text_layer_set_text_alignment(textBox->textLayer, GTextAlignmentCenter);
        if(textBox->scrollLayer)
            scroll_layer_add_child(textBox->scrollLayer, (Layer*)textBox->textLayer);
        else
            layer_add_child(textBox->mainLayer, (Layer*)textBox->textLayer);
		text_layer_set_text(textBox->textLayer, initialText);
		textBox->initialized = true;

		layer_set_update_proc(textBox->mainLayer, TextBoxUpdateProc);
	}
	layer_add_child(layer, textBox->mainLayer);
}

void FreeTextBox(TextBox *textBox)
{
    if(!textBox)
        return;
    
	if(textBox->initialized)
	{
		layer_destroy(textBox->mainLayer);
		text_layer_destroy(textBox->textLayer);
        if(textBox->scrollLayer)
            scroll_layer_destroy(textBox->scrollLayer);
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

void TextBox_ScrollUp(TextBox *textBox)
{
    DEBUG_LOG("ScrollUp");
    if(textBox->scrollLayer)
    {
        GPoint offset = scroll_layer_get_content_offset(textBox->scrollLayer);
        offset.y += WINDOW_ROW_HEIGHT;
        scroll_layer_set_content_offset(textBox->scrollLayer, offset, true);
    }
}

void TextBox_ScrollDown(TextBox *textBox)
{
    DEBUG_LOG("ScrollDown");
    if(textBox->scrollLayer)
    {
        GPoint offset = scroll_layer_get_content_offset(textBox->scrollLayer);
        offset.y -= WINDOW_ROW_HEIGHT;
        scroll_layer_set_content_offset(textBox->scrollLayer, offset, true);
    }
}
