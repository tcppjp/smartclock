#include "rca.h"
#include <functional>
#include <string>
#include <tuple>
#include <utility>
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "Adafruit_STMPE610.h"

#include "pindef.h"
#include "geometry.h"

namespace
{
	Adafruit_ILI9341 tft {
		pins::TftScreenCS, pins::TftScreenDC
	};

	Adafruit_STMPE610 touch {
		pins::TftResistiveTouchPanelCS
	};

	class UIElement;

	bool g_dirty = true;
	bool g_dirtyRectValid = false;
	SCRect g_dirtyRect;
	UIElement *g_rootElement = nullptr;
	UIElement *g_mouseCaptureElement = nullptr;
	UIElement *g_keyboardFocusElement = nullptr;
	std::function<void()> g_invoke;

	namespace colors
	{
		uint16_t buttonFace = tft.color565(192, 192, 192);
		uint16_t buttonHighlight = tft.color565(255, 255, 255);
		uint16_t buttonShadow = tft.color565(128, 128, 128);
		uint16_t buttonDarkShadow = tft.color565(64, 64, 64);
		uint16_t buttonText = tft.color565(0, 0, 0);
		uint16_t fieldFace = tft.color565(255, 255, 255);
		uint16_t fieldText = tft.color565(0, 0, 0);
		uint16_t selectionColor = tft.color565(0, 0, 128);
		uint16_t selectionText = tft.color565(255, 255, 255);
		uint16_t caret = tft.color565(255, 0, 0);
	}

	void ui_invalidate(const SCRect &rt);
	void ui_validate();

	struct UIElementPaintContext
	{
		int16_t layer;
		int16_t firstLayer;
		SCRect dirtyRect;
	};

	class UIElement
	{
		UIElement *_parent;
		UIElement *_prev, *_next; // cyclic
		UIElement *_child; // first child
		SCRect _bounds;
		bool _needsUpdate;
	public:
		UIElement() :
			_parent(nullptr),
			_prev(nullptr),
			_next(nullptr),
			_child(nullptr),
			_bounds {0, 0, 0, 0},
			_needsUpdate {true}
		{ } 
		~UIElement()
		{
			if (g_keyboardFocusElement == this) {
				g_keyboardFocusElement = nullptr;
			}
		}
		const SCRect &bounds() { return _bounds; }
		void setBounds(const SCRect &newBounds)
		{
			invalidateRegion();
			_bounds = newBounds;
			invalidateRegion();
		}
		const SCSize &size() { return _bounds.size; }
		void setSize(const SCSize &newSize)
		{
			setBounds(SCRect {bounds().loc, newSize});
		}
		const SCPoint &location() { return _bounds.loc; }
		void setLocation(const SCPoint &newLoc)
		{
			setBounds(SCRect {newLoc, bounds().size});
		}
		void removeFromParent()
		{
			assert(_parent);
			invalidateRegion();
			if (_prev == this) {
				assert(_next == this);
				assert(_parent->_child == this);
				_parent->_child = nullptr;
			} else {
				if (_parent->_child == this) {
					_parent->_child = _next;
				}
				assert(_next->_prev == this);
				assert(_prev->_next == this);
				_next->_prev = _prev;
				_prev->_next = _next;
			}
			_prev = _next = nullptr;
			_parent = nullptr;
		}
		void appendChild(UIElement *o)
		{
			assert(o);
			if (o->_parent) {
				o->removeFromParent();
			}
			if (_child == nullptr) {
				_child = o;
				o->_prev = o->_next = o;
			} else {
				o->_prev = _child->_prev;
				o->_next = _child;
				_child->_prev->_next = o;
				_child->_prev = o;
			}
			o->_parent = this;
			o->invalidate();
		}
		bool hasKeyboardFocus()
		{
			return g_keyboardFocusElement == this;
		}
		void focus()
		{
			if (g_keyboardFocusElement == this) {
				return;
			}
			auto *lastFocusElem = g_keyboardFocusElement;
			g_keyboardFocusElement = this;
			if (lastFocusElem) {
				lastFocusElem->lostFocus();
			}
			gotFocus();
		}
		void blur()
		{
			if (g_keyboardFocusElement != this) {
				return;
			}
			g_keyboardFocusElement = nullptr;
			lostFocus();
		}
		SCPoint screenLocation() const
		{
			if (_parent) {
				return _parent->screenLocation() + _bounds.loc;
			} else {
				return _bounds.loc;
			}
		}
		SCRect screenBounds() const
		{
			return SCRect {screenLocation(), _bounds.size};
		}
		void invalidate()
		{
			_needsUpdate = true;
			g_dirty = true;
		}
		void invalidateRegion()
		{
			ui_invalidate(screenBounds());
		}
		template <class T>
		void forEachChild(T cb)
		{
			if (_child == nullptr) {
				return;
			}
			UIElement *first = _child;
			UIElement *e = first;
			do {
				cb(e);
				e = e->_next;
			} while (e != first);
		}
		void paintPreprocess(const SCPoint &offset,
			UIElementPaintContext &ctx)
		{
			SCRect scrBounds = _bounds.translated(offset);
			if (!scrBounds.intersectsWith(ctx.dirtyRect)) {
				return;
			}
			if (scrBounds.covers(ctx.dirtyRect)) {
				ctx.firstLayer = ctx.layer;
			}
			ctx.layer += 1;
			forEachChild([&] (UIElement *e) {
				e->paintPreprocess(scrBounds.loc, ctx);
			});
		}
		void paint(const SCPoint &offset,
			UIElementPaintContext &ctx,
			bool forceNeedsUpdate = false)
		{
			SCRect scrBounds = _bounds.translated(offset);
			// FIXME: repainting system is messed up; needs refactoring
			if ((scrBounds.intersectsWith(ctx.dirtyRect) &&
				ctx.layer >= ctx.firstLayer ) ||
				_needsUpdate || forceNeedsUpdate) {
				clientPaint(scrBounds);
			}
			ctx.layer += 1;
			forEachChild([&] (UIElement *e) {
				e->paint(scrBounds.loc, ctx, _needsUpdate || forceNeedsUpdate);
			});
			_needsUpdate = false;
		}
		virtual void clientPaint(const SCRect &scrBounds)
		{ }
		virtual void clientMouseDown(const SCPoint &mouseLoc)
		{ }
		virtual void clientMouseMove(const SCPoint &mouseLoc)
		{ }
		virtual void clientMouseUp(const SCPoint &mouseLoc)
		{ }
		virtual void gotFocus()
		{ }
		virtual void lostFocus()
		{ }
		virtual void clientCharInput(char c)
		{ }
	};	

	void ui_invalidate(const SCRect &rt)
	{
		SCRect r = rt.intersectionWith(
			SCRect {
				0, 0, 240, 320
			}
		);
		if (r.empty()) {
			return;
		}
		if (g_dirtyRectValid) {
			g_dirtyRect = g_dirtyRect.unionWith(r);
		} else {
			g_dirtyRect = r;
			g_dirtyRectValid = true;
		}
		g_dirty = true;
	}

	inline void ui_invalidate()
	{
		ui_invalidate(SCRect {0, 0, 4096, 4096});
	}

	void ui_validate()
	{
		if (!g_dirty) {
			return;
		}
		SCRect rt = g_dirtyRect;
		g_dirty = false;
		if (!g_dirtyRectValid) {
			rt = SCRect {0, 0, 0, 0};
		}
		g_dirtyRectValid = false;

		UIElementPaintContext ctx;
		ctx.dirtyRect = rt;
		ctx.layer = 0;
		ctx.firstLayer = 0;
		g_rootElement->paintPreprocess(SCPoint {0, 0}, ctx);
		ctx.layer = 0;
		g_rootElement->paint(SCPoint {0, 0}, ctx);
	}

	bool g_mousePressed = false;
	SCPoint g_mouseLocation;

	// These variables are updated in the interrupt context
	volatile bool g_imousePressed = false;
	volatile SCPoint g_imouseLocation;

	void ui_updateMouse()
	{
		loc_cpu();
		bool latestMousePressed = g_imousePressed;
		g_mouseLocation = SCPoint { g_imouseLocation.x, g_imouseLocation.y };
		unl_cpu();

		if (!g_mousePressed && latestMousePressed) {
			SCPoint relpt = g_mouseLocation;
			UIElement *e = g_rootElement;
			while (true) {
				relpt -= e->location();

				UIElement *hitChild = nullptr;
				e->forEachChild([&] (UIElement *child) {
					if (child->bounds().contains(relpt)) {
						hitChild = child;
					}
				});

				if (hitChild) {
					e = hitChild;
				} else {
					g_mouseCaptureElement = e;
					break;
				}
			}

			g_mouseCaptureElement->clientMouseDown(g_mouseLocation - 
				g_mouseCaptureElement->screenLocation());
			g_mousePressed = true;
		}

		if (g_mousePressed && !latestMousePressed) {
			if (g_mouseCaptureElement) {
				g_mouseCaptureElement->clientMouseUp(
					g_mouseLocation - 
					g_mouseCaptureElement->screenLocation());
				g_mouseCaptureElement = nullptr;
			}
			g_mousePressed = false;
		}

		if (g_mousePressed) {
			g_mouseCaptureElement->clientMouseMove(g_mouseLocation - 
				g_mouseCaptureElement->screenLocation());
		}
	}

	class StaticTextStorage
	{
		const char *_text;
	public:
		StaticTextStorage() :
			_text("yay!")
		{ }
		/** WARNING: text not copied! */
		void setText(const char *text) { _text = text; }
		const char *text() { return _text; }
	};

	class HeapTextStorage
	{
		std::string _text;
	public:
		HeapTextStorage()
		{ }
		void setText(const char *text) { _text = text; }
		void setText(const std::string &text) { _text = text.c_str(); }
		const char *text() { return _text.c_str(); }
	};

	template <class TextStor = StaticTextStorage>
	class LabelUIElement : public UIElement, public TextStor
	{
	public:
		LabelUIElement() { }
		void clientPaint(const SCRect &scrBounds) override
		{
			int x = scrBounds.loc.x, y = scrBounds.loc.y;
			tft.setTextSize(2);
			tft.setTextColor(colors::buttonText);
			tft.setCursor(x, y);
			tft.println(this->text());
		}
	};

	template <class TextStor = StaticTextStorage>
	class ButtonUIElement : public UIElement, public TextStor
	{
		bool _pressed;
		std::function<void()> _onActivated;
	public:
		ButtonUIElement() :
			_pressed(false)
		{ }
		void setOnActivated(std::function<void()> fn)
		{
			_onActivated = fn;
		}
		void clientPaint(const SCRect &scrBounds) override
		{
			int x = scrBounds.loc.x, y = scrBounds.loc.y;
			int w = scrBounds.size.w, h = scrBounds.size.h;

			tft.fillRect(x + 2, y + 2, w - 4, h - 4, colors::buttonFace);

			uint16_t palette[4];
			if (_pressed) {
				palette[0] = colors::buttonDarkShadow;
				palette[1] = colors::buttonShadow;
				palette[2] = colors::buttonFace;
				palette[3] = colors::buttonHighlight;
			} else {
				palette[0] = colors::buttonFace;
				palette[1] = colors::buttonHighlight;
				palette[2] = colors::buttonShadow;
				palette[3] = colors::buttonDarkShadow;
			}
			tft.fillRect(x, y, w, 1, palette[0]);
			tft.fillRect(x, y + 1, 1, h - 2, palette[0]);
			tft.fillRect(x + 1, y + 1, w - 2, 1, palette[1]);
			tft.fillRect(x + 1, y + 2, 1, h - 4, palette[1]);
			tft.fillRect(x + w - 1, y + 1, 1, h - 1, palette[3]);
			tft.fillRect(x, y + h - 1, w - 1, 1, palette[3]);
			tft.fillRect(x + w - 2, y + 2, 1, h - 4, palette[2]);
			tft.fillRect(x + 1, y + h - 2, w - 4, 1, palette[2]);

			tft.setTextSize(2);
			tft.setTextColor(colors::buttonText, colors::buttonFace);
			int16_t tx, ty; uint16_t tw, th;
			tft.getTextBounds(const_cast<char*>(this->text()), 0, 0, &tx, &ty, &tw, &th);
			tft.setCursor(x + (w - tw - tx) / 2, y + (h - th - ty) / 2);
			tft.println(this->text());
		}
		void clientMouseDown(const SCPoint &mouseLoc) override
		{
			_pressed = true;
			invalidate();
		}
		void clientMouseMove(const SCPoint &mouseLoc) override
		{
		}
		void clientMouseUp(const SCPoint &mouseLoc) override
		{
			_pressed = false;
			invalidate();

			if (SCRect{SCPoint{0, 0}, size()}.contains(mouseLoc)) {
				if (_onActivated) {
					_onActivated();
				}
			}
		}
	};

	template <class TextStor = HeapTextStorage>
	class EntryUIElement : public UIElement, public TextStor
	{
		int8_t _markIndex, _selIndex;
		bool _isPassword;

		bool canFitText(const char *s)
		{
			if (*s == 0) {
				return true;
			}
			tft.setTextSize(2);

			int16_t tx, ty; uint16_t tw, th;
			tft.getTextBounds(const_cast<char*>(s), 0, 0, &tx, &ty, &tw, &th);
			return tx + tw < size().w - 8;
		}

		int charIndexAtLocation(const SCPoint& p)
		{
			tft.setTextSize(2);

			std::string str = this->text();
			if (p.x < 0) {
				return 0;
			}

			int px = p.x - 4;

			int16_t tx, ty; uint16_t tw, th;
			tft.getTextBounds(const_cast<char*>(str.c_str()), 0, 0, &tx, &ty, &tw, &th);
			if (px > tx + tw) {
				return static_cast<int> (str.size());
			}

			int lastRight = 0;
			for (size_t i = 0; i < str.size(); ++i) {
				char c = str[i + 1];
				if (i + 1 < str.size()) {
					str[i + 1] = 0;
				}
				tft.getTextBounds(const_cast<char*>(str.c_str()), 0, 0, &tx, &ty, &tw, &th);
				int right = tx + tw;
				if (i + 1 < str.size()) {
					str[i + 1] = c;
				}

				if (px < (right + lastRight) / 2) {
					return static_cast<int> (i);
				}

				lastRight = right;
			}

			return static_cast<int> (str.size());
		}
	public:
		EntryUIElement() :
			_markIndex(0),
			_selIndex(0),
			_isPassword(false)
		{ }
		void setIsPassword(bool p)
		{
			_isPassword = p;
			invalidate();
		}
		void setSelectionRange(int mark, int sel = -1)
		{
			_markIndex = mark;
			_selIndex = sel >= 0 ? sel : mark;
			invalidate();
		}
		std::pair<int, int> selectionRange()
		{
			return std::make_pair<int, int>(
				SCMin(_markIndex, _selIndex),
				SCMax(_markIndex, _selIndex)
			);
		}
		void clientPaint(const SCRect &scrBounds) override
		{
			int x = scrBounds.loc.x, y = scrBounds.loc.y;
			int w = scrBounds.size.w, h = scrBounds.size.h;

			const char *str = this->text();
			if (_isPassword) {
				str = "********************" + (20 - strlen(str));
			}

			tft.fillRect(x + 2, y + 2, w - 4, h - 4, colors::fieldFace);

			uint16_t palette[4];
			palette[0] = colors::buttonDarkShadow;
			palette[1] = colors::buttonShadow;
			palette[2] = colors::buttonFace;
			palette[3] = colors::buttonHighlight;

			tft.fillRect(x, y, w, 1, palette[0]);
			tft.fillRect(x, y + 1, 1, h - 2, palette[0]);
			tft.fillRect(x + 1, y + 1, w - 2, 1, palette[1]);
			tft.fillRect(x + 1, y + 2, 1, h - 4, palette[1]);
			tft.fillRect(x + w - 1, y + 1, 1, h - 1, palette[3]);
			tft.fillRect(x, y + h - 1, w - 1, 1, palette[3]);
			tft.fillRect(x + w - 2, y + 2, 1, h - 4, palette[2]);
			tft.fillRect(x + 1, y + h - 2, w - 4, 1, palette[2]);

			tft.setTextSize(2);
			tft.setCursor(x + 4, y + 4);
			auto range = selectionRange();
			int i = 0;
			int curX = x + 4;
			while (*str) {
				if (i >= range.first && i < range.second) {
					tft.setTextColor(colors::selectionText, colors::selectionColor);
				} else {
					tft.setTextColor(colors::fieldText, colors::fieldFace);
				}
				tft.write(*str);
				++str; ++i;
				if (i == range.first) {
					curX = tft.getCursorX();
				}
			}

			if (hasKeyboardFocus() &&
				range.first == range.second) {
				tft.fillRect(curX, y + 4, 1, h - 8, colors::caret);
				tft.fillRect(curX - 2, y + 4, 2, 1, colors::caret);
				tft.fillRect(curX + 1, y + 4, 2, 1, colors::caret);
				tft.fillRect(curX - 2, y + h - 4, 2, 1, colors::caret);
				tft.fillRect(curX + 1, y + h - 4, 2, 1, colors::caret);
			}
		}
		void clientMouseDown(const SCPoint &mouseLoc) override
		{
			setSelectionRange(charIndexAtLocation(mouseLoc));
			invalidate();
			focus();
		}
		void clientMouseMove(const SCPoint &mouseLoc) override
		{
			_selIndex = charIndexAtLocation(mouseLoc);
			invalidate();
		}
		void clientMouseUp(const SCPoint &mouseLoc) override
		{
		}
		void clientCharInput(char c) override
		{
			std::string str = this->text();
			std::string str2;
			auto range = selectionRange();
			if (range.second != range.first) {
				str2 = str.substr(range.second);
				str.resize(range.first);
			}
			switch (c) {
				case 8:
					if (range.second == range.first && range.first > 0) {
						str.resize(range.first - 1);
						range.first -= 1;
					}
					break;
				case 13:
					// TODO: handle return key
					break;
				default:
					str.push_back(c);
					range.first += 1;
					break;
			}

			str += str2;
			if (!canFitText(str.c_str())) {
				return;
			}

			this->setText(str);
			setSelectionRange(range.first);
			invalidate();
		}
	};

	constexpr const char *softwareKeyboardLines[][3] = {
		{
			"qwertyuiop",
			"asdfghjkl",
			"zxcvbnm"
		},
		{
			"QWERTYUIOP",
			"ASDFGHJKL",
			"ZXCVBNM"
		},
		{
			"1234567890",
			"-/:;()&@\"",
			".,?!'"
		},
		{
			"[]{}#%^*+=",
			"_\\|~<>$:)(",
			".,?!'"
		}
	};

	class SoftwareKeyboardUIElement : public UIElement
	{
		ButtonUIElement<> keys[26];
		char keystrs[26][2];

		ButtonUIElement<> shiftKey;
		ButtonUIElement<> symbolKey;
		ButtonUIElement<> backSpaceKey;
		ButtonUIElement<> spaceBar;
		ButtonUIElement<> enterKey;

		int8_t currentPlane;
		void updateKeys()
		{
			for (int i = 0; i < 10; ++i) {
				keystrs[i][0] = softwareKeyboardLines[currentPlane][0][i];
				keystrs[i][1] = 0;
			}
			for (int i = 0; i < 9; ++i) {
				keystrs[i + 10][0] = softwareKeyboardLines[currentPlane][1][i];
				keystrs[i + 10][1] = 0;
			}
			for (int i = 0; i < 7; ++i) {
				keystrs[i + 19][0] = softwareKeyboardLines[currentPlane][2][i];
				keystrs[i + 19][1] = 0;
			}
			switch (currentPlane) {
				case 0:
					symbolKey.setText("123");
					shiftKey.setText("^");
					break;
				case 1:
					symbolKey.setText("123");
					shiftKey.setText("v");
					break;
				case 2:
					symbolKey.setText("ABC");
					shiftKey.setText("#+");
					break;
				case 3:
					symbolKey.setText("ABC");
					shiftKey.setText("12");
					break;
			}
		}

		void sendChar(char c)
		{
			if (g_keyboardFocusElement) {
				g_keyboardFocusElement->clientCharInput(c);
			}
		}

		void sendBackSpace()
		{
			if (g_keyboardFocusElement) {
				g_keyboardFocusElement->clientCharInput(8);
			}
		}
	public:
		SoftwareKeyboardUIElement() :
			currentPlane(0)
		{
			setBounds(SCRect {0, 155, 240, 165});
			for (int i = 0; i < 10; ++i) {
				keys[i].setBounds(SCRect { 
					static_cast<int16_t>(i * 23 + 5), 5, 23, 35 
				});
				keys[i].setText(keystrs[i]);
			}
			for (int i = 10; i < 19; ++i) {
				keys[i].setBounds(SCRect { 
					static_cast<int16_t>((i - 10) * 23 + 15), 45, 23, 35 
				});
				keys[i].setText(keystrs[i]);
			}
			for (int i = 19; i < 26; ++i) {
				keys[i].setBounds(SCRect { 
					static_cast<int16_t>((i - 19) * 23 + 38), 85, 23, 35 
				});
				keys[i].setText(keystrs[i]);
			}
			shiftKey.setBounds(SCRect { 5, 85, 28, 35 });
			shiftKey.setText("^");
			backSpaceKey.setBounds(SCRect { 204, 85, 31, 35 });
			backSpaceKey.setText("<X");
			symbolKey.setBounds(SCRect { 5, 125, 40, 35});
			spaceBar.setBounds(SCRect { 50, 125, 140, 35 });
			spaceBar.setText("");
			enterKey.setBounds(SCRect { 195, 125, 40, 35 });
			enterKey.setText("ENT");
			updateKeys();
			for (auto &e: keys) {
				appendChild(&e);
				auto *ep = &e;
				e.setOnActivated([this, ep] {
					sendChar(ep->text()[0]);
				});
			}
			appendChild(&shiftKey);
			appendChild(&symbolKey);
			appendChild(&spaceBar);
			appendChild(&backSpaceKey);
			appendChild(&enterKey);
			shiftKey.setOnActivated([&] {
				currentPlane ^= 1;
				updateKeys();
				invalidate();
			});
			symbolKey.setOnActivated([&] {
				currentPlane = (currentPlane ^ 2) & ~1;
				updateKeys();
				invalidate();
			});
			spaceBar.setOnActivated([&] {
				sendChar(' ');
			});
			backSpaceKey.setOnActivated([&] {
				sendBackSpace();
			});
			enterKey.setOnActivated([&] {
				sendChar('\n');
			});
		}
		void clientPaint(const SCRect &scrBounds) override
		{
			tft.fillRect(scrBounds.loc.x, scrBounds.loc.y,
				scrBounds.size.w, scrBounds.size.h, 
				colors::buttonFace);
		}
	};

	static SoftwareKeyboardUIElement g_softwareKeyboard;

	class SetupScreenUIElement : public UIElement
	{
		EntryUIElement<> field;
		ButtonUIElement<> button;
	public:
		SetupScreenUIElement()
		{
			setBounds(SCRect {0, 0, 240, 320});
			appendChild(&button);
			button.setBounds(SCRect {20, 30, 120, 40});
			button.setText("Yay!");

			appendChild(&field);
			field.setBounds(SCRect {5, 5, 230, 24});
			field.setText("hoge");

			appendChild(&g_softwareKeyboard);
		} 
		void clientPaint(const SCRect &scrBounds) override
		{
			tft.fillRect(scrBounds.loc.x, scrBounds.loc.y,
				scrBounds.size.w, scrBounds.size.h, 
				colors::buttonFace);
		}
	};

	static SetupScreenUIElement g_setupScreen;
}


void ui_setup(void)
{
	pinMode(pins::TftScreenCS, OUTPUT);
	pinMode(pins::TftResistiveTouchPanelCS, OUTPUT);
	digitalWrite(pins::TftScreenCS, HIGH);
	digitalWrite(pins::TftResistiveTouchPanelCS, HIGH);

	Serial.println("ui_setup: initializing TFT");
	tft.begin();

	Serial.println("ui_setup: initializing touch screen");
	if (!touch.begin()) {
		Serial.println("ui_setup: something went horribly wrong! I just gtfo");
	}

	Serial.println("ui_setup: printing diag info");
	// diagnostics info (we don't need this but it makes the startup
	// routine about 20% cooler)
	uint8_t x = tft.readcommand8(ILI9341_RDMODE);
	Serial.print("Display Power Mode: 0x"); Serial.println(x, HEX);
	x = tft.readcommand8(ILI9341_RDMADCTL);
	Serial.print("MADCTL Mode: 0x"); Serial.println(x, HEX);
	x = tft.readcommand8(ILI9341_RDPIXFMT);
	Serial.print("Pixel Format: 0x"); Serial.println(x, HEX);
	x = tft.readcommand8(ILI9341_RDIMGFMT);
	Serial.print("Image Format: 0x"); Serial.println(x, HEX);
	x = tft.readcommand8(ILI9341_RDSELFDIAG);
	Serial.print("Self Diagnostic: 0x"); Serial.println(x, HEX); 

	Serial.print("Touch screen version: 0x"); Serial.println(touch.getVersion(), HEX); 

	g_rootElement = &g_setupScreen;
	ui_invalidate();

	Serial.println("ui_setup: initial screen created");

	act_tsk(UI_TASK);
	sta_cyc(UI_TOUCH_CYC);
}

extern "C" void ui_task(intptr_t exinf)
{
	while (true) {

		// (copied from Adafruit_STMPE610s.cpp)
    	SPI.setClockDivider(84);
      	SPI.setDataMode(SPI_MODE1);

		// these were in the interrupt context, but now here!
		if (!touch.bufferEmpty()) {
			TS_Point p;
			do {
				p = touch.getPoint();
			} while (!touch.bufferEmpty());
			SCPoint pt {
				static_cast<int16_t> (map(p.x, 150, 3800, 0, 240)), 
				static_cast<int16_t> (map(p.y, 130, 4000, 0, 320))
			};
			g_imouseLocation.x = pt.x;
			g_imouseLocation.y = pt.y;
			g_imousePressed = true;
		}
		if (!touch.touched()) {
			g_imousePressed = false;
		}

		// (copied from Adafruit_ILI9341.cpp)
    	SPI.setClockDivider(11); // 8-ish MHz (full! speed!)
      	SPI.setDataMode(SPI_MODE0);

      	{
			wai_sem(UI_INVOKE_SEM);
			std::function<void()> cb;
			cb.swap(g_invoke);
			sig_sem(UI_INVOKE_SEM);
			if (cb) {
				cb();
				sig_sem(UI_INVOKE_DONE_SEM);
			}
		}

		ui_updateMouse();
		if (!g_dirty) {
			slp_tsk();
		}

		ui_validate();
	}
}

extern "C" void ui_checkTouch(intptr_t exinf)
{
	iwup_tsk(UI_TASK);
}

void ui_showSplash()
{
	Serial.println("ui_showSplash: initializing TFT");
	tft.begin();
	
	uint16_t bgColor = tft.color565(255, 255, 255);
	uint16_t bdrColor = tft.color565(0, 0, 255);
	uint16_t txtColor = tft.color565(0, 0, 0);
	tft.fillRect(0, 0, tft.width(), 20, bdrColor);
	tft.fillRect(0, 20, 20, tft.height() - 40, bdrColor);
	tft.fillRect(tft.width() - 20, 20, 20, tft.height() - 40, bdrColor);
	tft.fillRect(0, tft.height() - 20, tft.width(), 20, bdrColor);
	tft.fillRect(20, 20, tft.width() - 40, tft.height() - 40, bgColor);
	tft.setTextColor(txtColor);
	tft.setTextSize(2);
	tft.setCursor(30, 100);
	tft.println("HOWDY!");
	tft.setTextSize(1);
	tft.setCursor(30, 180);
	tft.println("You'll never be tardy again");
}

void ui_showFatalError(const char *msg)
{
	uint16_t bgColor = tft.color565(255, 255, 255);
	uint16_t bdrColor = tft.color565(255, 0, 0);
	uint16_t txtColor = tft.color565(0, 0, 0);
	tft.fillRect(0, 0, tft.width(), 20, bdrColor);
	tft.fillRect(0, 20, 20, tft.height() - 40, bdrColor);
	tft.fillRect(tft.width() - 20, 20, 20, tft.height() - 40, bdrColor);
	tft.fillRect(0, tft.height() - 20, tft.width(), 20, bdrColor);
	tft.fillRect(20, 20, tft.width() - 40, tft.height() - 40, bgColor);
	tft.setTextColor(txtColor);
	tft.setTextSize(2);
	tft.setCursor(30, 100);
	tft.println("FATAL ERROR");
	tft.setTextSize(1);
	tft.setCursor(30, 180);
	tft.println(msg);
}

void ui_invokeSync(std::function<void()> cb)
{
	wai_sem(UI_INVOKE_SEM);
	g_invoke = cb;
	sig_sem(UI_INVOKE_SEM);
	wup_tsk(UI_TASK);
	wai_sem(UI_INVOKE_DONE_SEM);
}
