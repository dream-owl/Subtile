#include "subtile.h"

#include <algorithm>
#include <exception>

#include <cmath>
#include <cstdint>
#include <cstring>
#include <cassert>

stMaterial::stMaterial(stLabel const& label) : label(label)
{
    for(size_t i = 0; i < Values.size(); i++)
        values[i] = Values[i].limit;
}

struct stSubtile::stTile
{
    stTile()=default;
    stTile(int32_t altitude, float x, float y, int16_t m = 0, int16_t b = 0) : location(altitude, x, y) , material(m) , behavior(b) {}

    stLocation location;
    int16_t material;
    int16_t behavior;
};

struct stSubtile::stIsland
{
    stIsland()=default;
    stIsland(int32_t x, int32_t y, int32_t z) : xyz{ x, y, z } , life(stSettings::LifetimeIsland) , tiles(0) ,
                                                bounds(stLocation(z, x, y), stLocation(z, x + stSettings::SizeIsland, y + stSettings::SizeIsland)) {}

    int32_t xyz[3];
    int32_t life;
    uint8_t tiles;
    stTile storage[stSettings::StorageIsland];
    stBounds bounds;
};

stSubtile::stSubtile(std::string const& guid) : m_guid(guid) , m_directory(guid + '/') , m_materialsPool(stSettings::PoolMaterial) ,
                                                m_behaviorsPool(stSettings::PoolBehavior) , m_islandsPool(stSettings::PoolIsland)
{
    m_materialsPool.acquire() = stMaterial("blank");
    m_behaviorsPool.acquire() = stBehavior();
}

stSubtile::~stSubtile()
{

}

void stSubtile::parse(stPackage const& package)
{

}

void stSubtile::parse(stRequest const& request)
{
    int16_t materialHandle = 0;
    int16_t behaviorHandle = 0;

    for(size_t i = 0; i < m_materials.size(); i++)
    {
        if(!m_materials[i]->instanced() && (m_materials[i]->label == request.material.label))
        {
            materialHandle = static_cast<int16_t>(i);
            i = m_materials.size();
        }
    }

    if(materialHandle == 0)
        material(materialHandle, true) = request.material;

    stIsland* const islandPointer = island(request.location.altitude, request.location.position.x, request.location.position.y);
    islandPointer->storage[islandPointer->tiles++] = stTile(request.location.altitude, request.location.position.x, request.location.position.y, materialHandle, behaviorHandle);
}

void stSubtile::visit(stVisitor& visitor)
{
    for(stMaterial* material : m_materials)
        visitor.onMaterial(*material);

    for(stIsland* island : m_islands)
    {
        stLocation location;
        location.altitude = island->xyz[2];

        for(uint8_t i = 0; i < island->tiles; i++)
        {
            location.position = island->storage[i].location.position;
            visitor.onTile(location, *material(island->storage[i].material), *behavior(island->storage[i].behavior));
        }
    }
}

void stSubtile::visit(stVisitor& visitor, stBounds const& bounds)
{
    for(stIsland* island : m_islands)
    {
        if(bounds.overlaps(island->bounds))
        {
            stLocation location;
            location.altitude = island->xyz[2];

            for(uint8_t i = 0; i < island->tiles; i++)
            {
                if(bounds.overlaps(island->storage[i].location))
                {
                    location.position = island->storage[i].location.position;
                    visitor.onTile(location, *material(island->storage[i].material), *behavior(island->storage[i].behavior));
                }
            }
        }
    }
}

stPackage const* stSubtile::step()
{
    return nullptr;
}

stPackage const* stSubtile::pack()
{
    return nullptr;
}

stMaterial* stSubtile::material(int16_t handle)
{
    return &m_materialsPool.at(std::abs(handle));
}

stMaterial& stSubtile::material(int16_t& handle, bool dynamic)
{
    if(dynamic)
    {
        if(handle == 0)
        {
            std::cout << "Creating new material.\n";

            m_materialsPool.acquire(handle);
            m_materialsPool.at(handle) = stMaterial("new");

            m_materials.push_back(&m_materialsPool.at(handle));
        }
        else if(handle > 0)
        {
            std::cout << "Creating material instance.\n";

            int16_t newHandle = 0;

            m_materialsPool.acquire(newHandle);
            m_materialsPool.at(newHandle) = m_materialsPool.at(handle);
            m_materialsPool.at(newHandle).values[stMaterial::Base] = handle;

            handle = -newHandle;
        }
        else
        {
            std::cout << "Removing material instance.\n";

            int16_t const newHandle = m_materialsPool.at(std::abs(handle)).values[stMaterial::Base].value;

            m_materialsPool.release(std::abs(handle));

            handle = newHandle;
        }
    }

    return m_materialsPool.at(std::abs(handle));
}

stBehavior* stSubtile::behavior(int16_t handle)
{
    return &m_behaviorsPool.at(std::abs(handle));
}

stBehavior& stSubtile::behavior(int16_t& handle, bool dynamic)
{
    return m_behaviorsPool.at(std::abs(handle));
}

stSubtile::stIsland* stSubtile::island(int32_t altitude, int32_t x, int32_t y)
{
    static auto constexpr division = [](int32_t& value, int32_t const unit) {
        value = ((value < 0) ? (value / unit - 1) : (value / unit)) * unit;
    };

    division(x, stSettings::SizeIsland);
    division(y, stSettings::SizeIsland);

    for(stIsland* island : m_islands)
    {
        if((island->xyz[0] == x) && (island->xyz[1] == y) && (island->xyz[2] == altitude))
        {
            island->life = stSettings::LifetimeIsland;
            return island;
        }
    }

    m_islands.emplace_back(&m_islandsPool.acquire());
   *m_islands.back() = stIsland(x, y, altitude);

    return m_islands.back();
}

stSubtile::stIsland* stSubtile::island(int32_t altitude, float x, float y)
{
    return island(altitude, static_cast<int32_t>(x), static_cast<int32_t>(y));
}
