#include <GL/glew.h>
#include "windowManager.h"
#include "Updatable.h"
#include "Renderable.h"
#include "collisionable.h"
#include "postProcessManager.h"
#include "input.h"

WindowManager::WindowManager(int virtualWidth, int virtualHeight, bool fullscreen, RenderChain* renderChain, int fsaa)
: virtualSize(virtualWidth, virtualHeight), renderChain(renderChain), fullscreen(fullscreen), fsaa(fsaa)
{
    srand(time(NULL));
    windowHasFocus = true;
    min_aspect_ratio = float(virtualWidth) / float(virtualHeight);
    allow_virtual_resize = false;

    create();
    glewInit();
}

WindowManager::~WindowManager()
{
}

void WindowManager::render()
{
    // Clear the window
    window.clear(sf::Color(20, 20, 20));
    
    //Call the first item of the rendering chain.
    renderChain->render(window);

    // Display things on screen
    window.display();
}

void WindowManager::close()
{
    window.close();
}

void WindowManager::setFullscreen(bool new_fullscreen)
{
    if (fullscreen == new_fullscreen)
        return;
    fullscreen = new_fullscreen;
    create();
}
void WindowManager::setFSAA(int new_fsaa)
{
    if (fsaa == new_fsaa)
        return;
    fsaa = new_fsaa;
    create();
}

sf::Vector2f WindowManager::mapPixelToCoords(const sf::Vector2i& point) const
{
    return window.mapPixelToCoords(point);
}

sf::Vector2i WindowManager::mapCoordsToPixel(const sf::Vector2f& point) const
{
    return window.mapCoordsToPixel(point);
}

void WindowManager::create()
{
    // Create the window of the application
    int windowWidth = virtualSize.x;
    int windowHeight = virtualSize.y;
    
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    if (fullscreen)
    {
        windowWidth = desktop.width;
        windowHeight = desktop.height;
    }else{
        unsigned int scale = 2;
        while(windowWidth * scale < desktop.width && windowHeight * scale < desktop.height)
            scale += 1;
        windowWidth *= scale - 1;
        windowHeight *= scale - 1;
        
        while(windowWidth >= int(desktop.width) || windowHeight >= int(desktop.height) - 100)
        {
            windowWidth *= 0.9;
            windowHeight *= 0.9;
        }
    }

    sf::ContextSettings context_settings(24, 8, fsaa, 2, 0);
    if (fullscreen)
        window.create(sf::VideoMode(windowWidth, windowHeight, 32), "Game", sf::Style::Fullscreen, context_settings);
    else
        window.create(sf::VideoMode(windowWidth, windowHeight, 32), "Game", sf::Style::Default, context_settings);
    sf::ContextSettings settings = window.getSettings();
    LOG(INFO) << "OpenGL version: " << settings.majorVersion << "." << settings.minorVersion;
    window.setVerticalSyncEnabled(false);
    window.setFramerateLimit(60);
    window.setMouseCursorVisible(false);
    setupView();
}

void WindowManager::setupView()
{
    sf::Vector2f window_size = sf::Vector2f(window.getSize());
    if (window_size.x / window_size.y > min_aspect_ratio)
    {
        if (allow_virtual_resize)
            virtualSize.x = virtualSize.y * (window_size.x / window_size.y);
        
        float aspect = window_size.y * float(virtualSize.x) / float(virtualSize.y) / window_size.x;
        float offset = 0;//0.5 - 0.5 * aspect;
        sf::View view(sf::Vector2f(virtualSize.x/2,virtualSize.y/2), sf::Vector2f(virtualSize.x, virtualSize.y));
        view.setViewport(sf::FloatRect(offset, 0, aspect, 1));
        window.setView(view);
    }else{
        virtualSize.x = virtualSize.y * min_aspect_ratio;
        
        float aspect = window_size.x / window_size.y * float(virtualSize.y) / float(virtualSize.x);
        float offset = 0.5 - 0.5 * aspect;
        sf::View view(sf::Vector2f(virtualSize.x/2,virtualSize.y/2), sf::Vector2f(virtualSize.x, virtualSize.y));
        view.setViewport(sf::FloatRect(0, offset, 1, aspect));
        window.setView(view);
    }
}
