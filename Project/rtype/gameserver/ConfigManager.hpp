//
// Created by doom on 28/01/18.
//

#ifndef RTYPE_SERVER_CONFIGMANAGER_HPP
#define RTYPE_SERVER_CONFIGMANAGER_HPP

#include <unordered_map>
#include <rapidjson/document.h>
#include <SFML/Graphics/Rect.hpp>

namespace rtype
{
    class ConfigManager
    {
    private:
        void _parseConfigInside(const rapidjson::Document &doc, const std::string &faction);
        bool _loadFactionConfig(const std::string &faction) noexcept;

    public:
        bool loadConfig() noexcept
        {
            if (!_loadFactionConfig("Bheet") || !_loadFactionConfig("Kooy") || !_loadFactionConfig("Maul"))
                return false;
            return true;
        }

        const sf::IntRect &factionToBounds(const std::string &faction) const noexcept
        {
            return _factionToBounds.at(faction);
        }

    private:
        std::unordered_map<std::string, sf::IntRect> _factionToBounds;
    };
}

#endif //RTYPE_SERVER_CONFIGMANAGER_HPP
