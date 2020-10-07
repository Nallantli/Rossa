#include "../ruota/Ruota.h"

#include <SDL2/SDL.h>
#include <algorithm>

RUOTA_LIB_HEADER

namespace libsdl
{
	static bool SDL_INITIALIZED = false;

	static std::map<Uint32, Symbol> registered = {};

	struct Shape
	{
		int base_x, base_y;
		static hashcode_t id_count;
		hashcode_t id;
		short r, g, b, a;
		Shape(int base_x, int base_y, short r, short b, short g, short a) : base_x(base_x), base_y(base_y), r(r), g(g), b(b), a(a), id(id_count++) {}
		virtual void draw(SDL_Renderer *renderer, const Token *token) const = 0;

		void setColor(short r, short g, short b, short a)
		{
			this->r = r;
			this->g = g;
			this->b = b;
			this->a = a;
		}

		void setPosition(int base_x, int base_y)
		{
			this->base_x = base_x;
			this->base_y = base_y;
		}

		bool operator==(const Shape &s) const
		{
			return id == s.id;
		}
	};

	hashcode_t Shape::id_count = 0;

	struct Rectangle : public Shape
	{
		int width;
		int height;

		Rectangle(int base_x, int base_y, int width, int height, short r, short g, short b, short a) : Shape(base_x, base_y, r, g, b, a), width(width), height(height) {}

		void draw(SDL_Renderer *renderer, const Token *token) const override
		{
			SDL_Rect temp = {base_x, base_y, width, height};
			if (SDL_SetRenderDrawColor(renderer, r, g, b, a) < 0)
				throw RuotaError((boost::format("Error setting shape color: %1%") % SDL_GetError()).str(), *token);
			if (SDL_RenderFillRect(renderer, &temp) < 0)
				throw RuotaError((boost::format("Error drawing shape: %1%") % SDL_GetError()).str(), *token);
		}
	};

	struct Line : public Shape
	{
		int x2;
		int y2;

		Line(int base_x, int base_y, int x2, int y2, short r, short g, short b, short a) : Shape(base_x, base_y, r, g, b, a), x2(x2), y2(y2) {}

		void draw(SDL_Renderer *renderer, const Token *token) const override
		{
			if (SDL_SetRenderDrawColor(renderer, r, g, b, a) < 0)
				throw RuotaError((boost::format("Error setting shape color: %1%") % SDL_GetError()).str(), *token);
			if (SDL_RenderDrawLine(renderer, base_x, base_y, x2, y2) < 0)
				throw RuotaError((boost::format("Error drawing shape: %1%") % SDL_GetError()).str(), *token);
		}
	};

	struct Point : public Shape
	{
		Point(int base_x, int base_y, short r, short g, short b, short a) : Shape(base_x, base_y, r, g, b, a) {}

		void draw(SDL_Renderer *renderer, const Token *token) const override
		{
			if (SDL_SetRenderDrawColor(renderer, r, g, b, a) < 0)
				throw RuotaError((boost::format("Error setting shape color: %1%") % SDL_GetError()).str(), *token);
			if (SDL_RenderDrawPoint(renderer, base_x, base_y) < 0)
				throw RuotaError((boost::format("Error drawing shape: %1%") % SDL_GetError()).str(), *token);
		}
	};

	struct Graphics
	{
		Uint32 windowID;
		SDL_Renderer *renderer = NULL;
		SDL_Window *window = NULL;
		std::vector<Symbol> shapes;

		Graphics(const std::string &title, int width, int height, const Token *token)
		{
			window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
			if (window == NULL)
				throw RuotaError((boost::format("Failure to initialize window: %1%") % SDL_GetError()).str(), *token);

			this->windowID = SDL_GetWindowID(window);

			renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
			if (renderer == NULL)
				throw RuotaError((boost::format("Failure to initialize renderer: %1%") % SDL_GetError()).str(), *token);
		}

		void addShape(const Symbol &shape)
		{
			shapes.push_back(shape);
		}

		void removeShape(const Symbol &shape)
		{
			shapes.erase(std::remove_if(shapes.begin(), shapes.end(), [shape](const Symbol &s) {
							 return s.getPointer(NULL) == shape.getPointer(NULL);
						 }),
						 shapes.end());
		}

		void draw(const Token *token)
		{
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
			SDL_RenderClear(renderer);
			for (auto &s : shapes)
				reinterpret_cast<Shape *>(s.getPointer(token))->draw(renderer, token);
			SDL_RenderPresent(renderer);
		}

		~Graphics()
		{
			registered.erase(windowID);
			SDL_DestroyWindow(window);
			SDL_DestroyRenderer(renderer);
		}
	};

	RUOTA_EXT_SYM(_window_init, args, token, hash)
	{
		if (!SDL_INITIALIZED)
		{
			SDL_INITIALIZED = true;
			if (SDL_Init(SDL_INIT_VIDEO) < 0)
				throw RuotaError((boost::format("Failure to initialize SDL: %1%") % SDL_GetError()).str(), *token);
		}

		Graphics *g = new Graphics(args[0].getString(token), args[1].getNumber(token).getLong(), args[2].getNumber(token).getLong(), token);
		std::vector<Symbol> v = {Symbol(static_cast<std::shared_ptr<void>>(g)), Symbol(CNumber::Long(g->windowID))};
		return Symbol(v);
	}

	RUOTA_EXT_SYM(_event_poll, args, token, hash)
	{
		SDL_Event e;
		std::map<hashcode_t, Symbol> data;
		if (SDL_PollEvent(&e))
		{
			data[hash.hashString("type")] = Symbol(CNumber::Long(e.type));
			switch (e.type)
			{
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
				std::map<hashcode_t, Symbol> keysym;
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
				data[hash.hashString("text")] = Symbol(std::string(e.edit.text));
				data[hash.hashString("start")] = Symbol(CNumber::Long(e.edit.start));
				data[hash.hashString("length")] = Symbol(CNumber::Long(e.edit.length));
				return Symbol(data);
			case SDL_TEXTINPUT:
				data[hash.hashString("timestamp")] = Symbol(CNumber::Long(e.text.timestamp));
				data[hash.hashString("windowID")] = Symbol(CNumber::Long(e.text.windowID));
				data[hash.hashString("window")] = registered[e.text.windowID];
				data[hash.hashString("text")] = Symbol(std::string(e.text.text));
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
				if (e.drop.file != NULL)
				{
					data[hash.hashString("file")] = Symbol(std::string(e.drop.file));
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

	RUOTA_EXT_SYM(_window_add, args, token, hash)
	{
		auto g = reinterpret_cast<Graphics *>(args[0].getPointer(token));

		g->addShape(args[1]);
		return Symbol();
	}

	RUOTA_EXT_SYM(_window_remove, args, token, hash)
	{
		auto g = reinterpret_cast<Graphics *>(args[0].getPointer(token));

		g->removeShape(args[1]);
		return Symbol();
	}

	RUOTA_EXT_SYM(_shape_setColor, args, token, hash)
	{
		auto shape = reinterpret_cast<Shape *>(args[0].getPointer(token));

		short r = args[1].getNumber(token).getLong();
		short g = args[2].getNumber(token).getLong();
		short b = args[3].getNumber(token).getLong();
		short a = args[4].getNumber(token).getLong();

		shape->setColor(r, g, b, a);

		return Symbol();
	}

	RUOTA_EXT_SYM(_shape_setPosition, args, token, hash)
	{
		auto shape = reinterpret_cast<Shape *>(args[0].getPointer(token));

		int x = args[1].getNumber(token).getLong();
		int y = args[2].getNumber(token).getLong();

		shape->setPosition(x, y);

		return Symbol();
	}

	RUOTA_EXT_SYM(_rect_init, args, token, hash)
	{
		int x = args[0].getNumber(token).getLong();
		int y = args[1].getNumber(token).getLong();
		int width = args[2].getNumber(token).getLong();
		int height = args[3].getNumber(token).getLong();

		short r = args[4].getNumber(token).getLong();
		short g = args[5].getNumber(token).getLong();
		short b = args[6].getNumber(token).getLong();
		short a = args[7].getNumber(token).getLong();

		auto rect = std::make_shared<Rectangle>(x, y, width, height, r, g, b, a);
		return Symbol(static_cast<std::shared_ptr<void>>(rect));
	}

	RUOTA_EXT_SYM(_line_init, args, token, hash)
	{
		int x1 = args[0].getNumber(token).getLong();
		int y1 = args[1].getNumber(token).getLong();
		int x2 = args[2].getNumber(token).getLong();
		int y2 = args[3].getNumber(token).getLong();

		short r = args[4].getNumber(token).getLong();
		short g = args[5].getNumber(token).getLong();
		short b = args[6].getNumber(token).getLong();
		short a = args[7].getNumber(token).getLong();

		auto line = std::make_shared<Line>(x1, y1, x2, y2, r, g, b, a);
		return Symbol(static_cast<std::shared_ptr<void>>(line));
	}

	RUOTA_EXT_SYM(_point_init, args, token, hash)
	{
		int x = args[0].getNumber(token).getLong();
		int y = args[1].getNumber(token).getLong();

		short r = args[2].getNumber(token).getLong();
		short g = args[3].getNumber(token).getLong();
		short b = args[4].getNumber(token).getLong();
		short a = args[5].getNumber(token).getLong();

		auto point = std::make_shared<Point>(x, y, r, g, b, a);
		return Symbol(static_cast<std::shared_ptr<void>>(point));
	}

	RUOTA_EXT_SYM(_window_update, args, token, hash)
	{
		auto g = reinterpret_cast<Graphics *>(args[0].getPointer(token));
		g->draw(token);
		return Symbol();
	}
} // namespace libsdl