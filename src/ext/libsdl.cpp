#include "../ruota/Ruota.h"

#ifdef __unix__
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#else
#include <SDL.h>
#include <SDL_image.h>
#endif
#include <algorithm>

RUOTA_LIB_HEADER

namespace libsdl
{
	static bool SDL_INITIALIZED = false;

	static std::map<Uint32, Symbol> registered = {};

	struct Image
	{
	private:
		SDL_Surface *loaded = NULL;
		SDL_Texture *image = NULL;

	public:
		Image(const string &path, const Token *token, const short &r, const short &g, const short &b)
		{
			loaded = IMG_Load(path.c_str());
			if (loaded == NULL)
				throw RuotaError((boost::format("Texture file `%1%` loading error: %2%") % path % IMG_GetError()).str(), *token);
			SDL_SetColorKey(loaded, SDL_TRUE, SDL_MapRGB(loaded->format, r, g, b));
		}

		Image(const string &path, const Token *token)
		{
			loaded = IMG_Load(path.c_str());
			if (loaded == NULL)
				throw RuotaError((boost::format("Texture file `%1%` loading error: %2%") % path % IMG_GetError()).str(), *token);
		}

		SDL_Texture *getImage(SDL_Renderer *renderer, const Token *token)
		{
			if (image == NULL) {
				image = SDL_CreateTextureFromSurface(renderer, loaded);
				if (image == NULL)
					throw RuotaError((boost::format("Cannot create renderable image: ") % SDL_GetError()).str(), *token);
				SDL_FreeSurface(loaded);
			}
			return image;
		}

		~Image()
		{
			if (loaded != NULL)
				SDL_FreeSurface(loaded);
			if (image != NULL)
				SDL_DestroyTexture(image);
		}
	};

	struct Shape
	{
		short r, g, b, a;
		const hash_ull id;
		static hash_ull id_count;

		Shape(const short &r, const short &b, const short &g, const short &a) : r(r), g(g), b(b), a(a), id(id_count++)
		{}

		virtual void draw(SDL_Renderer *renderer, const Token *token, const int &x, const int &y) = 0;

		void setColor(const short &r, const short &g, const short &b, const short &a)
		{
			this->r = r;
			this->g = g;
			this->b = b;
			this->a = a;
		}

		bool operator==(const Shape &s) const
		{
			return id == s.id;
		}
	};

	hash_ull Shape::id_count = 0;

	struct Sizable : public Shape
	{
		int width;
		int height;

		Sizable(const int &width, const int &height, const short &r, const short &g, const short &b, const short &a) : Shape(r, g, b, a), width(width), height(height)
		{}

		void setSize(const int &width, const int &height)
		{
			this->width = width;
			this->height = height;
		}

		void setWidth(const int &width)
		{
			this->width = width;
		}

		void setHeight(const int &height)
		{
			this->height = height;
		}
	};

	struct Rotatable : public Sizable
	{
		double angle = 0;
		SDL_Point *center = NULL;
		SDL_Rect *clip = NULL;

		Rotatable(const int &width, const int &height, const short &r, const short &g, const short &b, const short &a) : Sizable(width, height, r, g, b, a)
		{}

		void setAngle(const double &angle)
		{
			this->angle = angle;
		}

		void setCenter(const int &x, const int &y)
		{
			if (center != NULL)
				SDL_free(center);
			center = new SDL_Point({ x, y });
		}

		void deCenter()
		{
			if (center != NULL)
				SDL_free(center);
			center = NULL;
		}

		void setClip(const int &cx, const int &cy, const int &cwidth, const int &cheight)
		{
			if (clip != NULL)
				SDL_free(clip);
			clip = new SDL_Rect({ cx, cy, cwidth, cheight });
		}

		void deClip()
		{
			if (clip != NULL)
				SDL_free(clip);
			clip = NULL;
		}

		~Rotatable()
		{
			if (center != NULL)
				SDL_free(center);
			if (clip != NULL)
				SDL_free(clip);
		}
	};

	struct Rectangle : public Sizable
	{
		Rectangle(const int &width, const int &height, const short &r, const short &g, const short &b, const short &a) : Sizable(width, height, r, g, b, a)
		{}

		void draw(SDL_Renderer *renderer, const Token *token, const int &x, const int &y) override
		{
			SDL_Rect temp = { x, y, width, height };
			if (SDL_SetRenderDrawColor(renderer, r, g, b, a) < 0)
				throw RuotaError((boost::format("Error setting shape color: %1%") % SDL_GetError()).str(), *token);
			if (SDL_RenderFillRect(renderer, &temp) < 0)
				throw RuotaError((boost::format("Error drawing shape: %1%") % SDL_GetError()).str(), *token);
		}
	};

	struct Line : public Sizable
	{
		Line(const int &width, const int &height, const short &r, const short &g, const short &b, const short &a) : Sizable(width, height, r, g, b, a)
		{}

		void draw(SDL_Renderer *renderer, const Token *token, const int &x, const int &y) override
		{
			if (SDL_SetRenderDrawColor(renderer, r, g, b, a) < 0)
				throw RuotaError((boost::format("Error setting shape color: %1%") % SDL_GetError()).str(), *token);
			if (SDL_RenderDrawLine(renderer, x, y, x + width, y + height) < 0)
				throw RuotaError((boost::format("Error drawing shape: %1%") % SDL_GetError()).str(), *token);
		}
	};

	struct Point : public Shape
	{
		Point(const short &r, const short &g, const short &b, const short &a) : Shape(r, g, b, a)
		{}

		void draw(SDL_Renderer *renderer, const Token *token, const int &x, const int &y) override
		{
			if (SDL_SetRenderDrawColor(renderer, r, g, b, a) < 0)
				throw RuotaError((boost::format("Error setting shape color: %1%") % SDL_GetError()).str(), *token);
			if (SDL_RenderDrawPoint(renderer, x, y) < 0)
				throw RuotaError((boost::format("Error drawing shape: %1%") % SDL_GetError()).str(), *token);
		}
	};

	struct Texture : public Rotatable
	{
		Symbol image;

		Texture(const Symbol &image, const int &width, const int &height, const short &r, const short &g, const short &b) : Rotatable(width, height, r, g, b, 0), image(image)
		{}

		void setImage(const Symbol &image)
		{
			this->image = image;
		}

		void draw(SDL_Renderer *renderer, const Token *token, const int &x, const int &y) override
		{
			auto img = COERCE_PTR(image.getPointer(token), Image)->getImage(renderer, token);
			SDL_SetTextureColorMod(img, r, g, b);
			SDL_Rect temp = { x, y, width, height };
			SDL_RenderCopyEx(renderer, img, clip, &temp, angle, center, SDL_FLIP_NONE);
		}
	};

	struct Renderer
	{
		Uint32 windowID;
		SDL_Renderer *renderer = NULL;
		std::vector<std::pair<Symbol, std::pair<int, int>>> shapes;

		Renderer(SDL_Window *window, const Token *token)
		{
			renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
			if (renderer == NULL)
				throw RuotaError((boost::format("Failure to initialize renderer: %1%") % SDL_GetError()).str(), *token);
		}

		void addShape(const Symbol &shape, const int &x, const int &y)
		{
			shapes.push_back({ shape, {x, y} });
		}

		void clearAll()
		{
			shapes.clear();
		}

		void draw(const Token *token)
		{
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
			SDL_RenderClear(renderer);
			for (auto &s : shapes)
				COERCE_PTR(s.first.getPointer(token), Shape)->draw(renderer, token, s.second.first, s.second.second);
			SDL_RenderPresent(renderer);
		}

		~Renderer()
		{
			SDL_DestroyRenderer(renderer);
		}
	};

	struct Window
	{
		Uint32 windowID;
		SDL_Window *window = NULL;

		Window(const string &title, const int &width, const int &height, const Token *token)
		{
			window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
			if (window == NULL)
				throw RuotaError((boost::format("Failure to initialize window: %1%") % SDL_GetError()).str(), *token);

			this->windowID = SDL_GetWindowID(window);
		}

		std::shared_ptr<Renderer> getRenderer(const Token *token)
		{
			auto g = std::make_shared<Renderer>(window, token);
			return g;
		}

		~Window()
		{
			registered.erase(windowID);
			SDL_DestroyWindow(window);
		}
	};

	RUOTA_EXT_SYM(_window_init, args, token, hash)
	{
		if (!SDL_INITIALIZED) {
			SDL_INITIALIZED = true;
			if (SDL_Init(SDL_INIT_VIDEO) < 0)
				throw RuotaError((boost::format("Failure to initialize SDL: %1%") % SDL_GetError()).str(), *token);

			int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
			if (!(IMG_Init(imgFlags) & imgFlags))
				throw RuotaError((boost::format("Failure to initialize SDL_image: %1%") % IMG_GetError()).str(), *token);
		}

		auto w = std::make_shared<Window>(args[0].getString(token), args[1].getNumber(token).getLong(), args[2].getNumber(token).getLong(), token);
		std::vector<Symbol> v = { Symbol(static_cast<std::shared_ptr<void>>(w)), Symbol(CNumber::Long(w->windowID)) };
		return Symbol(v);
	}

	RUOTA_EXT_SYM(_event_poll, args, token, hash)
	{
		SDL_Event e;
		std::map<hash_ull, Symbol> data;
		if (SDL_PollEvent(&e)) {
			data[hash.hashString("type")] = Symbol(CNumber::Long(e.type));
			switch (e.type) {
				case SDL_WINDOWEVENT:
					data[hash.hashString("timestamp")] = Symbol(CNumber::Long(e.window.timestamp));
					data[hash.hashString("windowID")] = Symbol(CNumber::Long(e.window.windowID));
					data[hash.hashString("window")] = registered[e.window.windowID];
					data[hash.hashString("event")] = Symbol(CNumber::Long(e.window.event));
					data[hash.hashString("data1")] = Symbol(CNumber::Long(e.window.data1));
					data[hash.hashString("data2")] = Symbol(CNumber::Long(e.window.data2));
					return Symbol(data);
				case SDL_KEYDOWN:
				case SDL_KEYUP:
				{
					data[hash.hashString("timestamp")] = Symbol(CNumber::Long(e.key.timestamp));
					data[hash.hashString("windowID")] = Symbol(CNumber::Long(e.key.windowID));
					data[hash.hashString("window")] = registered[e.key.windowID];
					data[hash.hashString("state")] = Symbol(CNumber::Long(e.key.state));
					data[hash.hashString("repeat")] = Symbol(CNumber::Long(e.key.repeat));
					std::map<hash_ull, Symbol> keysym;
					keysym[hash.hashString("scancode")] = Symbol(CNumber::Long(e.key.keysym.scancode));
					keysym[hash.hashString("sym")] = Symbol(CNumber::Long(e.key.keysym.sym));
					keysym[hash.hashString("mod")] = Symbol(CNumber::Long(e.key.keysym.mod));
					data[hash.hashString("keysym")] = Symbol(keysym);
					return Symbol(data);
				}
				case SDL_TEXTEDITING:
					data[hash.hashString("timestamp")] = Symbol(CNumber::Long(e.edit.timestamp));
					data[hash.hashString("windowID")] = Symbol(CNumber::Long(e.edit.windowID));
					data[hash.hashString("window")] = registered[e.edit.windowID];
					data[hash.hashString("text")] = Symbol(string(e.edit.text));
					data[hash.hashString("start")] = Symbol(CNumber::Long(e.edit.start));
					data[hash.hashString("length")] = Symbol(CNumber::Long(e.edit.length));
					return Symbol(data);
				case SDL_TEXTINPUT:
					data[hash.hashString("timestamp")] = Symbol(CNumber::Long(e.text.timestamp));
					data[hash.hashString("windowID")] = Symbol(CNumber::Long(e.text.windowID));
					data[hash.hashString("window")] = registered[e.text.windowID];
					data[hash.hashString("text")] = Symbol(string(e.text.text));
					return Symbol(data);
				case SDL_MOUSEMOTION:
					data[hash.hashString("timestamp")] = Symbol(CNumber::Long(e.motion.timestamp));
					data[hash.hashString("windowID")] = Symbol(CNumber::Long(e.motion.windowID));
					data[hash.hashString("window")] = registered[e.motion.windowID];
					data[hash.hashString("which")] = Symbol(CNumber::Long(e.motion.which));
					data[hash.hashString("state")] = Symbol(CNumber::Long(e.motion.state));
					data[hash.hashString("x")] = Symbol(CNumber::Long(e.motion.x));
					data[hash.hashString("y")] = Symbol(CNumber::Long(e.motion.y));
					data[hash.hashString("xrel")] = Symbol(CNumber::Long(e.motion.xrel));
					data[hash.hashString("yrel")] = Symbol(CNumber::Long(e.motion.yrel));
					return Symbol(data);
				case SDL_MOUSEBUTTONDOWN:
				case SDL_MOUSEBUTTONUP:
					data[hash.hashString("timestamp")] = Symbol(CNumber::Long(e.button.timestamp));
					data[hash.hashString("windowID")] = Symbol(CNumber::Long(e.button.windowID));
					data[hash.hashString("window")] = registered[e.button.windowID];
					data[hash.hashString("which")] = Symbol(CNumber::Long(e.button.which));
					data[hash.hashString("state")] = Symbol(CNumber::Long(e.button.state));
					data[hash.hashString("x")] = Symbol(CNumber::Long(e.button.x));
					data[hash.hashString("y")] = Symbol(CNumber::Long(e.button.y));
					data[hash.hashString("button")] = Symbol(CNumber::Long(e.button.button));
					data[hash.hashString("clicks")] = Symbol(CNumber::Long(e.button.clicks));
					return Symbol(data);
				case SDL_MOUSEWHEEL:
					data[hash.hashString("timestamp")] = Symbol(CNumber::Long(e.wheel.timestamp));
					data[hash.hashString("windowID")] = Symbol(CNumber::Long(e.wheel.windowID));
					data[hash.hashString("window")] = registered[e.wheel.windowID];
					data[hash.hashString("which")] = Symbol(CNumber::Long(e.wheel.which));
					data[hash.hashString("direction")] = Symbol(CNumber::Long(e.wheel.direction));
					data[hash.hashString("x")] = Symbol(CNumber::Long(e.wheel.x));
					data[hash.hashString("y")] = Symbol(CNumber::Long(e.wheel.y));
					return Symbol(data);
				case SDL_JOYAXISMOTION:
					data[hash.hashString("timestamp")] = Symbol(CNumber::Long(e.jaxis.timestamp));
					data[hash.hashString("which")] = Symbol(CNumber::Long(e.jaxis.which));
					data[hash.hashString("axis")] = Symbol(CNumber::Long(e.jaxis.axis));
					data[hash.hashString("value")] = Symbol(CNumber::Long(e.jaxis.value));
					return Symbol(data);
				case SDL_JOYBALLMOTION:
					data[hash.hashString("timestamp")] = Symbol(CNumber::Long(e.jball.timestamp));
					data[hash.hashString("which")] = Symbol(CNumber::Long(e.jball.which));
					data[hash.hashString("ball")] = Symbol(CNumber::Long(e.jball.ball));
					data[hash.hashString("xrel")] = Symbol(CNumber::Long(e.jball.xrel));
					data[hash.hashString("yrel")] = Symbol(CNumber::Long(e.jball.yrel));
					return Symbol(data);
				case SDL_JOYHATMOTION:
					data[hash.hashString("timestamp")] = Symbol(CNumber::Long(e.jhat.timestamp));
					data[hash.hashString("which")] = Symbol(CNumber::Long(e.jhat.which));
					data[hash.hashString("hat")] = Symbol(CNumber::Long(e.jhat.hat));
					data[hash.hashString("value")] = Symbol(CNumber::Long(e.jhat.value));
					return Symbol(data);
				case SDL_JOYBUTTONDOWN:
				case SDL_JOYBUTTONUP:
					data[hash.hashString("timestamp")] = Symbol(CNumber::Long(e.jbutton.timestamp));
					data[hash.hashString("which")] = Symbol(CNumber::Long(e.jbutton.which));
					data[hash.hashString("button")] = Symbol(CNumber::Long(e.jbutton.button));
					data[hash.hashString("state")] = Symbol(CNumber::Long(e.jbutton.state));
					return Symbol(data);
				case SDL_JOYDEVICEADDED:
				case SDL_JOYDEVICEREMOVED:
					data[hash.hashString("timestamp")] = Symbol(CNumber::Long(e.jdevice.timestamp));
					data[hash.hashString("which")] = Symbol(CNumber::Long(e.jdevice.which));
					return Symbol(data);
				case SDL_CONTROLLERAXISMOTION:
					data[hash.hashString("timestamp")] = Symbol(CNumber::Long(e.caxis.timestamp));
					data[hash.hashString("which")] = Symbol(CNumber::Long(e.caxis.which));
					data[hash.hashString("axis")] = Symbol(CNumber::Long(e.caxis.axis));
					data[hash.hashString("value")] = Symbol(CNumber::Long(e.caxis.value));
					return Symbol(data);
				case SDL_CONTROLLERBUTTONDOWN:
				case SDL_CONTROLLERBUTTONUP:
					data[hash.hashString("timestamp")] = Symbol(CNumber::Long(e.cbutton.timestamp));
					data[hash.hashString("which")] = Symbol(CNumber::Long(e.cbutton.which));
					data[hash.hashString("button")] = Symbol(CNumber::Long(e.cbutton.button));
					data[hash.hashString("state")] = Symbol(CNumber::Long(e.cbutton.state));
					return Symbol(data);
				case SDL_CONTROLLERDEVICEADDED:
				case SDL_CONTROLLERDEVICEREMOVED:
				case SDL_CONTROLLERDEVICEREMAPPED:
					data[hash.hashString("timestamp")] = Symbol(CNumber::Long(e.cdevice.timestamp));
					data[hash.hashString("which")] = Symbol(CNumber::Long(e.cdevice.which));
					return Symbol(data);
				case SDL_AUDIODEVICEADDED:
				case SDL_AUDIODEVICEREMOVED:
					data[hash.hashString("timestamp")] = Symbol(CNumber::Long(e.adevice.timestamp));
					data[hash.hashString("which")] = Symbol(CNumber::Long(e.adevice.which));
					data[hash.hashString("iscapture")] = Symbol(CNumber::Long(e.adevice.iscapture));
					return Symbol(data);
				case SDL_QUIT:
					data[hash.hashString("timestamp")] = Symbol(CNumber::Long(e.quit.timestamp));
					return Symbol(data);
				case SDL_FINGERMOTION:
				case SDL_FINGERDOWN:
				case SDL_FINGERUP:
					data[hash.hashString("timestamp")] = Symbol(CNumber::Long(e.tfinger.timestamp));
					data[hash.hashString("touchId")] = Symbol(CNumber::Long(e.tfinger.touchId));
					data[hash.hashString("fingerId")] = Symbol(CNumber::Long(e.tfinger.fingerId));
					data[hash.hashString("x")] = Symbol(CNumber::Double(e.tfinger.x));
					data[hash.hashString("y")] = Symbol(CNumber::Double(e.tfinger.y));
					data[hash.hashString("dx")] = Symbol(CNumber::Double(e.tfinger.dx));
					data[hash.hashString("dy")] = Symbol(CNumber::Double(e.tfinger.dy));
					data[hash.hashString("pressure")] = Symbol(CNumber::Double(e.tfinger.pressure));
					return Symbol(data);
				case SDL_MULTIGESTURE:
					data[hash.hashString("timestamp")] = Symbol(CNumber::Long(e.mgesture.timestamp));
					data[hash.hashString("touchId")] = Symbol(CNumber::Long(e.mgesture.touchId));
					data[hash.hashString("dTheta")] = Symbol(CNumber::Double(e.mgesture.dTheta));
					data[hash.hashString("dDist")] = Symbol(CNumber::Double(e.mgesture.dDist));
					data[hash.hashString("x")] = Symbol(CNumber::Double(e.mgesture.x));
					data[hash.hashString("y")] = Symbol(CNumber::Double(e.mgesture.y));
					data[hash.hashString("numFingers")] = Symbol(CNumber::Long(e.mgesture.numFingers));
					return Symbol(data);
				case SDL_DOLLARGESTURE:
				case SDL_DOLLARRECORD:
					data[hash.hashString("timestamp")] = Symbol(CNumber::Long(e.dgesture.timestamp));
					data[hash.hashString("touchId")] = Symbol(CNumber::Long(e.dgesture.touchId));
					data[hash.hashString("gestureId")] = Symbol(CNumber::Long(e.dgesture.gestureId));
					data[hash.hashString("numFingers")] = Symbol(CNumber::Long(e.dgesture.numFingers));
					data[hash.hashString("error")] = Symbol(CNumber::Double(e.dgesture.error));
					data[hash.hashString("x")] = Symbol(CNumber::Double(e.dgesture.x));
					data[hash.hashString("y")] = Symbol(CNumber::Double(e.dgesture.y));
					return Symbol(data);
				case SDL_DROPFILE:
				case SDL_DROPBEGIN:
				case SDL_DROPTEXT:
				case SDL_DROPCOMPLETE:
					data[hash.hashString("timestamp")] = Symbol(CNumber::Long(e.drop.timestamp));
					data[hash.hashString("windowID")] = Symbol(CNumber::Long(e.drop.windowID));
					data[hash.hashString("window")] = registered[e.drop.windowID];
					if (e.drop.file != NULL) {
						data[hash.hashString("file")] = Symbol(string(e.drop.file));
						SDL_free(e.drop.file);
					}
					return Symbol(data);
				default:
					return Symbol(data);
			}
		}

		data[hash.hashString("type")] = Symbol(CNumber());
		return Symbol(data);
	}

	RUOTA_EXT_SYM(_window_register, args, token, hash)
	{
		registered[args[0].getNumber(token).getLong()] = args[1];
		return Symbol();
	}

	RUOTA_EXT_SYM(_window_getRenderer, args, token, hash)
	{
		auto w = COERCE_PTR(
			args[0].getPointer(token),
			Window);

		return Symbol(static_cast<std::shared_ptr<void>>(w->getRenderer(token)));
	}

	RUOTA_EXT_SYM(_renderer_draw, args, token, hash)
	{
		auto g = COERCE_PTR(
			args[0].getPointer(token),
			Renderer);

		int x = args[2].getNumber(token).getLong();
		int y = args[3].getNumber(token).getLong();

		g->addShape(args[1], x, y);
		return Symbol();
	}

	RUOTA_EXT_SYM(_shape_setColor, args, token, hash)
	{
		auto shape = COERCE_PTR(
			args[0].getPointer(token),
			Shape);

		short r = args[1].getNumber(token).getLong();
		short g = args[2].getNumber(token).getLong();
		short b = args[3].getNumber(token).getLong();
		short a = args[4].getNumber(token).getLong();

		shape->setColor(r, g, b, a);

		return Symbol();
	}

	RUOTA_EXT_SYM(_rotatable_setAngle, args, token, hash)
	{
		auto rot = COERCE_PTR(
			args[0].getPointer(token),
			Rotatable);

		int angle = args[1].getNumber(token).getDouble();

		rot->setAngle(angle);

		return Symbol();
	}

	RUOTA_EXT_SYM(_rotatable_setCenter, args, token, hash)
	{
		auto rot = COERCE_PTR(
			args[0].getPointer(token),
			Rotatable);

		int x = args[1].getNumber(token).getLong();
		int y = args[2].getNumber(token).getLong();

		rot->setCenter(x, y);

		return Symbol();
	}

	RUOTA_EXT_SYM(_rotatable_deCenter, args, token, hash)
	{
		auto rot = COERCE_PTR(
			args[0].getPointer(token),
			Rotatable);

		rot->deCenter();

		return Symbol();
	}

	RUOTA_EXT_SYM(_rotatable_setClip, args, token, hash)
	{
		auto rot = COERCE_PTR(
			args[0].getPointer(token),
			Rotatable);

		int x = args[1].getNumber(token).getLong();
		int y = args[2].getNumber(token).getLong();
		int width = args[3].getNumber(token).getLong();
		int height = args[4].getNumber(token).getLong();

		rot->setClip(x, y, width, height);

		return Symbol();
	}

	RUOTA_EXT_SYM(_rotatable_deClip, args, token, hash)
	{
		auto rot = COERCE_PTR(
			args[0].getPointer(token),
			Rotatable);

		rot->deClip();

		return Symbol();
	}

	RUOTA_EXT_SYM(_rect_init, args, token, hash)
	{
		int width = args[0].getNumber(token).getLong();
		int height = args[1].getNumber(token).getLong();

		short r = args[2].getNumber(token).getLong();
		short g = args[3].getNumber(token).getLong();
		short b = args[4].getNumber(token).getLong();
		short a = args[5].getNumber(token).getLong();

		auto rect = std::make_shared<Rectangle>(width, height, r, g, b, a);
		return Symbol(static_cast<std::shared_ptr<void>>(rect));
	}

	RUOTA_EXT_SYM(_sizable_setSize, args, token, hash)
	{
		auto sizable = COERCE_PTR(
			args[0].getPointer(token),
			Sizable);

		int width = args[1].getNumber(token).getLong();
		int height = args[2].getNumber(token).getLong();

		sizable->setSize(width, height);
		return Symbol();
	}

	RUOTA_EXT_SYM(_sizable_setWidth, args, token, hash)
	{
		auto sizable = COERCE_PTR(
			args[0].getPointer(token),
			Sizable);

		int width = args[1].getNumber(token).getLong();

		sizable->setWidth(width);
		return Symbol();
	}

	RUOTA_EXT_SYM(_sizable_setHeight, args, token, hash)
	{
		auto sizable = COERCE_PTR(
			args[0].getPointer(token),
			Sizable);

		int height = args[1].getNumber(token).getLong();

		sizable->setHeight(height);
		return Symbol();
	}

	RUOTA_EXT_SYM(_line_init, args, token, hash)
	{
		int x2 = args[0].getNumber(token).getLong();
		int y2 = args[1].getNumber(token).getLong();

		short r = args[2].getNumber(token).getLong();
		short g = args[3].getNumber(token).getLong();
		short b = args[4].getNumber(token).getLong();
		short a = args[5].getNumber(token).getLong();

		auto line = std::make_shared<Line>(x2, y2, r, g, b, a);
		return Symbol(static_cast<std::shared_ptr<void>>(line));
	}

	RUOTA_EXT_SYM(_point_init, args, token, hash)
	{
		short r = args[0].getNumber(token).getLong();
		short g = args[1].getNumber(token).getLong();
		short b = args[2].getNumber(token).getLong();
		short a = args[3].getNumber(token).getLong();

		auto point = std::make_shared<Point>(r, g, b, a);
		return Symbol(static_cast<std::shared_ptr<void>>(point));
	}

	RUOTA_EXT_SYM(_image_init_nokey, args, token, hash)
	{
		string path = args[0].getString(token);

		auto image = std::make_shared<Image>(path, token);
		return Symbol(static_cast<std::shared_ptr<void>>(image));
	}

	RUOTA_EXT_SYM(_image_init_key, args, token, hash)
	{
		string path = args[0].getString(token);
		short r = args[1].getNumber(token).getLong();
		short g = args[2].getNumber(token).getLong();
		short b = args[3].getNumber(token).getLong();

		auto image = std::make_shared<Image>(path, token, r, g, b);
		return Symbol(static_cast<std::shared_ptr<void>>(image));
	}

	RUOTA_EXT_SYM(_texture_init, args, token, hash)
	{
		auto image = args[0];

		int width = args[1].getNumber(token).getLong();
		int height = args[2].getNumber(token).getLong();

		short r = args[3].getNumber(token).getLong();
		short g = args[4].getNumber(token).getLong();
		short b = args[5].getNumber(token).getLong();

		auto texture = std::make_shared<Texture>(image, width, height, r, g, b);
		return Symbol(static_cast<std::shared_ptr<void>>(texture));
	}

	RUOTA_EXT_SYM(_texture_setImage, args, token, hash)
	{
		auto texture = COERCE_PTR(
			args[0].getPointer(token),
			Texture);

		texture->setImage(args[1]);
		return Symbol();
	}

	RUOTA_EXT_SYM(_renderer_update, args, token, hash)
	{
		auto g = COERCE_PTR(
			args[0].getPointer(token),
			Renderer);

		g->draw(token);
		return Symbol();
	}

	RUOTA_EXT_SYM(_renderer_clear, args, token, hash)
	{
		auto g = COERCE_PTR(
			args[0].getPointer(token),
			Renderer);

		g->clearAll();
		return Symbol();
	}

	RUOTA_EXT_SYM(_renderer_flush, args, token, hash)
	{
		auto g = COERCE_PTR(
			args[0].getPointer(token),
			Renderer);

		g->draw(token);
		g->clearAll();
		return Symbol();
	}
}