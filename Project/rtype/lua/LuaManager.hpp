//
// Created by milerius on 22/01/18.
//

#ifndef RTYPE_LUAMANAGER_HPP
#define RTYPE_LUAMANAGER_HPP

#include <boost/filesystem.hpp>
#include <sol/sol.hpp>
#include <log/Logger.hpp>
#include <rtype/entity/ECS.hpp>

namespace fs = boost::filesystem;

namespace rtype::lua
{
    class LuaManager
    {
    public:
        LuaManager(rtype::EntityManager &em, fs::path scriptDirectoryPath) noexcept :
            _scriptDirectoryPath(std::move(scriptDirectoryPath)), _em(em)
        {
            _luaState.open_libraries();
            _createBasicFunctions();
            _registerType(rtype::Components{});
        }

        void loadAll() noexcept
        {
            _em.for_each<rtc::Lua>([this](Entity &ett) {
                auto str = ett.getComponent<rtc::Lua>().scriptName;
                auto self = ett.getComponent<rtc::Lua>().selfName;
                _luaState[self] = ett.getID();
                loadScript(str);
            });
        }

        bool loadScript(const std::string &fileName) noexcept
        {
            auto fullPath = _scriptDirectoryPath / fileName;
            try {
                _luaState.script_file(fullPath.string());
            } catch (const std::exception &e) {
                _log(logging::Debug) << fileName << ": " << e.what() << std::endl;
                return false;
            }
            return true;
        }

#ifndef USING_MSVC

        template <typename ReturnType = void, typename ...Args>
        ReturnType executeFunction(const std::string &funcName, Args &&...args)
        {
            if constexpr (std::is_void_v<ReturnType>) {
                _luaState[funcName](std::forward<Args>(args)...);
            } else {
                return _luaState[funcName](std::forward<Args>(args)...);
            }
        }

#else
        template<typename ...Args>
        void executeFunction(const std::string &funcName, Args &&...args)
        {
            _luaState[funcName](std::forward<Args>(args)...);
        }

        template<typename ReturnType, typename ... Args>
        ReturnType executeFunction(const std::string &funcName, Args &&...args)
        {
            ReturnType v = _luaState[funcName](std::forward<Args>(args)...);
            return v;
        }
#endif

        template <typename T>
        decltype(auto) operator[](const T &funcName)
        {
            return _luaState[funcName];
        }

    private:
        void _createBasicFunctions() noexcept
        {
            using namespace std::string_view_literals;
            _luaState.new_usertype<rtype::Entity>("Entity", "getID", &rtype::Entity::getID);
            _luaState.set_function("createEntity", [this]() { return _em.createEntity(); });
            _luaState.set_function("getEntity", [this](Entity::ID id) -> rtype::Entity & { return std::ref(_em[id]); });
            _luaState.set_function("getEntityC",
                                   [this](Entity::ID id) -> const rtype::Entity & { return std::ref(_em[id]); });
            _luaState.set_function("mark", [this](Entity::ID id) { _em[id].mark(); });
        }

        template <typename T>
        void _createType()
        {
            const auto table = std::tuple_cat(
                std::make_tuple(T::className()),
                T::memberMap()
            );

            std::apply(
                [this](auto &&...params) {
                    this->_luaState.new_usertype<T>(std::forward<decltype(params)>(params)...);
                }, table);

            if constexpr (meta::list::Contains<Components, T>::value) {
                _log(logging::Debug) << "Registering component " << T::className() << std::endl;
                _registerComponent<T>();
            }
        }

        template <typename T>
        void _registerComponent()
        {
            using namespace std::string_literals;
            _luaState["getEntityWith"s + T::className() + "Components"s] = [this] {
                std::vector<rtype::Entity *> _entities;
                _em.for_each<T>([&_entities](rtype::Entity &ett) {
                    _entities.push_back(&ett);
                });
                return _entities;
            };

            _luaState["Entity"]["get"s + T::className() + "Component"s] = [](rtype::Entity &self) {
                return std::ref(self.getComponent<T>());
            };

            _luaState["Entity"]["has"s + T::className() + "Component"s] = [](rtype::Entity &self) {
                return self.hasComponent<T>();
            };
        }

        template <typename ...Types>
        void _registerType(meta::TypeList<Types...>)
        {
            (_createType<Types>(), ...);
        }

    private:
        sol::state _luaState;
        fs::path _scriptDirectoryPath;
        rtype::EntityManager &_em;
        logging::Logger _log{"lua-manager", logging::Debug};
    };
}

#endif //RTYPE_LUAMANAGER_HPP
