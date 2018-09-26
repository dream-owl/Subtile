#ifndef SUBTILE_SUBTILE_H
#define SUBTILE_SUBTILE_H

#include "subcore.h"

class stMaterial
{
public:
    enum Values {
        Base,
        Volume,
        Density,
        Hitpoints,
        Temperature,
        Transparency
    };

    static constexpr std::array<stValueMeta, 6> Values =
    {
        stValueMeta("base", Base, 0),
        stValueMeta("volume", Volume, 100),
        stValueMeta("density", Density, 100),
        stValueMeta("hitpoints", Hitpoints, 100),
        stValueMeta("temperature", Temperature, 100),
        stValueMeta("transparency", Transparency, 100)
    };

    stMaterial()=default;
    stMaterial(stLabel const& label);

    bool instanced() const { return values[Base].value != Values[Base].limit; }
    bool destroyed() const { return values[Hitpoints].value <= 0; }

    stLabel label;
    stValue values[6];
};

class stBehavior
{
public:

};

class stRequest
{
public:
    stRequest()=default;
    stRequest(int32_t altitude, float x, float y, float r) : transform(altitude, x, y, r) , material("blank") , behavior() {}

    stTransform transform;
    stMaterial material;
    stBehavior behavior;
};

class stVisitor
{
public:
    virtual ~stVisitor() {}
    virtual void onMaterial(stMaterial const& material) {}
    virtual void onBehavior(stBehavior const& behavior) {}
    virtual void onTile(stTransform const& transform, stMaterial const& material, stBehavior const& behavior) {}
};

class stSubtile
{
public:
    stSubtile(std::string const& guid);
   ~stSubtile();

    void parse(stPackage const& package);
    void parse(stRequest const& request);

    void visit(stVisitor& visitor);
    void visit(stVisitor& visitor, stBounds const& bounds);

    stPackage const* step();
    stPackage const* pack();

private:
    struct stTile;
    struct stIsland;

    stMaterial* material(int16_t handle);
    stMaterial& material(int16_t& handle, bool dynamic);

    stBehavior* behavior(int16_t handle);
    stBehavior& behavior(int16_t& handle, bool dynamic);

    stIsland* island(int32_t altitude, int32_t x, int32_t y);
    stIsland* island(int32_t altitude, float x, float y);

    std::string const m_guid;
    std::string const m_directory;

    std::vector<stMaterial*> m_materials;
    std::vector<stIsland*> m_islands;

    stPool<stMaterial> m_materialsPool;
    stPool<stBehavior> m_behaviorsPool;
    stPool<stIsland> m_islandsPool;
};

#endif // SUBTILE_SUBTILE_H
