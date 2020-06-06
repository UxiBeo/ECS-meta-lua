#include <iostream>
#include "entt/entt.hpp"
#include <unordered_map>
#include "sol.hpp"

template<typename T>
T& Get(entt::registry* ecs, entt::entity entity, sol::state* lua)
{
	T& instance = registry.get_or_emplace<Type>(entity);
    (*lua)["Get"].get_or_create<sol::table>()[entt::type_info<T>::id()] = &instance;
	return instance;
}

template<typename T>
T& Set(entt::registry* ecs, entt::entity entity, const Type &instance)
{
   return ecs.emplace_or_replace<T>(entity, instance);
}
template<typename T>
void extend_meta_type() {
    entt::meta<T>()
        .template func<&Get<T>, entt::as_ref_t>("Get"_hs)
        .template func<&Set<T>>("Set"_hs);
}
void GetComponent(entt::registry* ecs, sol::state* lua, entt::entity entity, sol::variadic_args component)
{
    for (auto id : component)
    {
        if (auto type = entt::resolve_type(id.as<entt::id_type>()); type)
			type.func("get"_hs).invoke({}, std::ref(ecs), entity, lua);
    }
    
}
template<typename Type>
struct entt::type_index<Type> {
    static id_type value() ENTT_NOEXCEPT {
        static const id_type value = (extend_meta_type<Type>(), internal::type_index::next());
        return value;
    }
};
struct transform { int value; };
struct texture { int value; };
struct MagicSkill { int value; };
int main()
{
	entt::meta<transform>().type("transform"_hs);
	entt::meta<texture>().type("texture"_hs);
    entt::registry ecs;
    sol::state lua;
    lua.open_libraries();
    lua["ECS"] = &ecs;
    lua["lua"] = &lua;
    lua["GetComponent"] = &GetComponent;
    lua["transformID"] = entt::type_info<transform>::id();
    lua["textureID"] = entt::type_info<texture>::id();
    lua["MagicSkillID"] = entt::type_info<MagicSkill>::id();
    lua.new_usertype<transform>("transform",
        "value", &transform::value
        );
    lua.new_usertype<texture>("texture",
        "value", &texture::value
        );
    lua.new_usertype<MagicSkill>("MagicSkill",
        "value", &MagicSkill::value
        );
    const auto entity = ecs.create();
    ecs.emplace<transform>(entity, 1);
    ecs.emplace<texture>(entity, 2);
    lua["entity"] = entity;
	/*
    registry.visit(entity, [&](const auto component) {
        const auto type = entt::resolve_type(component);
        const auto any = type.func("get"_hs).invoke({}, std::ref(registry), entity);
        type.func("set"_hs).invoke({}, std::ref(registry), other, any);
    });
	*/
    auto meta_Resolve = entt::resolve<transform>();
    auto meta_Resolve_Type = entt::resolve_type(entt::type_info<transform>::id());
    auto resolveID = meta_Resolve.type_id();
    auto typeInfoID = entt::type_info<transform>::id();
    const auto& code = R"(  
        GetComponent(ECS, lua, entity, transformID, MagicSkillID, textureID)
        print(Get[transformID].value)
        print(Get[textureID].value)
        if Get[MagicSkillID] then 
            print(Get[MagicSkillID].value) 
        end 
        
	)";
    lua.script(code);
    
    return 1;
}
