/*
 * Copyright (C) 2006-2016 Christopho, Solarus - http://www.solarus-games.org
 *
 * Solarus is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Solarus is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef SOLARUS_ENTITIES_H
#define SOLARUS_ENTITIES_H

#include "solarus/Common.h"
#include "solarus/containers/Quadtree.h"
#include "solarus/entities/Camera.h"
#include "solarus/entities/CameraPtr.h"
#include "solarus/entities/EntityPtr.h"
#include "solarus/entities/EntityType.h"
#include "solarus/entities/Ground.h"
#include "solarus/entities/HeroPtr.h"
#include "solarus/entities/TilePtr.h"
#include "solarus/Transition.h"
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

namespace Solarus {

class Destination;
class Hero;
class Map;
class MapData;
class NonAnimatedRegions;
class Rectangle;
class Tileset;
class TilePattern;
struct TileInfo;

using EntityList = std::list<EntityPtr>;
using EntitySet = std::set<EntityPtr>;
using EntityVector = std::vector<EntityPtr>;
using ConstEntityVector = std::vector<ConstEntityPtr>;
using EntityTree = Quadtree<EntityPtr>;

/**
 * \brief Manages the whole content of a map.
 *
 * Each element of a map is called an entity and is an instance of
 * a subclass of Entity.
 * This class stores all entities of the current map:
 * the tiles, the hero, the enemies and all other entities.
 */
class SOLARUS_API Entities {

  public:

    // Creation and destruction.
    Entities(Game& game, Map& map);

    // Get entities.
    Hero& get_hero();
    const CameraPtr& get_camera() const;
    Ground get_tile_ground(int layer, int x, int y) const;
    EntityVector get_entities();
    const std::shared_ptr<Destination>& get_default_destination();

    // By name.
    EntityPtr get_entity(const std::string& name);
    EntityPtr find_entity(const std::string& name);

    EntityVector get_entities_with_prefix(const std::string& prefix);
    EntityVector get_entities_with_prefix_sorted(const std::string& prefix);
    EntityVector get_entities_with_prefix(EntityType type, const std::string& prefix);
    EntityVector get_entities_with_prefix_sorted(EntityType type, const std::string& prefix);
    bool has_entity_with_prefix(const std::string& prefix) const;

    // By type.
    EntitySet get_entities_by_type(EntityType type);
    EntityVector get_entities_by_type_sorted(EntityType type);
    EntitySet get_entities_by_type(EntityType type, int layer);

    // By type, template versions to avoid casts.
    template<typename T>
    std::set<std::shared_ptr<const T>> get_entities_by_type() const;
    template<typename T>
    std::set<std::shared_ptr<T>> get_entities_by_type();
    template<typename T>
    std::set<std::shared_ptr<const T>> get_entities_by_type(int layer) const;
    template<typename T>
    std::set<std::shared_ptr<T>> get_entities_by_type(int layer);

    // By coordinates.
    void get_entities_in_rectangle(const Rectangle& rectangle, ConstEntityVector& result) const;
    void get_entities_in_rectangle(const Rectangle& rectangle, EntityVector& result);
    void get_entities_in_rectangle_sorted(const Rectangle& rectangle, ConstEntityVector& result) const;
    void get_entities_in_rectangle_sorted(const Rectangle& rectangle, EntityVector& result);

    // By separator region.
    void get_entities_in_region(const Point& xy, EntityVector& result);
    void get_entities_in_region_sorted(const Point& xy, EntityVector& result);
    Rectangle get_region_box(const Point& point) const;

    // Handle entities.
    void create_entities(const MapData& data);
    void add_tile_info(const TileInfo& tile);
    void add_entity(const EntityPtr& entity);
    void add_tile(const TilePtr& entity);
    void remove_entity(Entity& entity);
    void remove_entity(const std::string& name);
    void remove_entities_with_prefix(const std::string& prefix);
    int get_entity_relative_z_order(const ConstEntityPtr& entity) const;
    void bring_to_front(Entity& entity);
    void bring_to_back(Entity& entity);
    void set_entity_layer(Entity& entity, int layer);
    void notify_entity_bounding_box_changed(Entity& entity);

    // Specific to some entity types.
    bool overlaps_raised_blocks(int layer, const Rectangle& rectangle) ;

    // Map events.
    void notify_map_started();
    void notify_map_opening_transition_finished();
    void notify_tileset_changed();
    void notify_map_finished();

    // Game loop.
    void set_suspended(bool suspended);
    void update();
    void draw();

  private:

    /**
     * \brief Mapping from layer to a type T.
     */
    template<typename T>
    using ByLayer = std::map<int, T>;

    /**
     * \brief Ordered list of entities to be drawn.
     */
    using EntitiesToDraw = std::vector<EntityPtr>;

    /**
     * \brief Internal fast cached information about the entity insertion order.
     */
    class ZCache {

      public:

        ZCache();

        int get_z(const ConstEntityPtr& entity) const;
        void add(const ConstEntityPtr& entity);
        void remove(const ConstEntityPtr& entity);
        void bring_to_front(const ConstEntityPtr& entity);
        void bring_to_back(const ConstEntityPtr& entity);

      private:

        std::unordered_map<ConstEntityPtr, int> z_values;
        int min;
        int max;
    };

    void initialize_layers();
    void set_tile_ground(int layer, int x8, int y8, Ground ground);
    void remove_marked_entities();
    void notify_entity_removed(Entity& entity);
    void update_crystal_blocks();

    // map
    Game& game;                                     /**< The game running this map */
    Map& map;                                       /**< The map */
    int map_width8;                                 /**< Number of 8x8 squares on a row of the map grid */
    int map_height8;                                /**< Number of 8x8 squares on a column of the map grid */

    // tiles
    int tiles_grid_size;                            /**< Number of 8x8 squares in the map
                                                     * (tiles_grid_size = map_width8 * map_height8) */
    ByLayer<std::vector<Ground>> tiles_ground;      /**< For each layer, list of size tiles_grid_size
                                                     * representing the ground property
                                                     * of each 8x8 square. */
    ByLayer<std::unique_ptr<NonAnimatedRegions>>
        non_animated_regions;                       /**< For each layer, all non-animated tiles are managed
                                                     * here for performance. */
    ByLayer<std::vector<TilePtr>>
        tiles_in_animated_regions;                  /**< For each layer, animated tiles and tiles overlapping them. */

    // dynamic entities
    HeroPtr hero;                                   /**< The hero, also stored in Game because
                                                     * it is kept when changing maps. */
    CameraPtr camera;                               /**< The visible area of the map. */

    std::map<std::string, EntityPtr>
        named_entities;                             /**< Entities identified by a name. */
    EntityList all_entities;                        /**< All map entities except tiles and the hero. */
    std::map<EntityType, ByLayer<EntitySet>>
        entities_by_type;                           /**< All map entities except tiles, by type and then layer. */

    EntityTree quadtree;                            /**< All map entities except tiles.
                                                     * Optimized for fast spatial search. */
    ByLayer<ZCache> z_caches;                       /**< For each layer, tracks the relative Z order of entities. */
    ByLayer<EntityVector>
        entities_drawn_not_at_their_position;       /**< For each layer, entities to draw even if there position
                                                     * is outside the camera. */
    ByLayer<EntitiesToDraw> entities_to_draw;       /**< For each layer, entities to be drawn at this cycle. */

    EntityList entities_to_remove;                  /**< List of entities that need to be removed right now. */

    std::shared_ptr<Destination>
        default_destination;                        /**< Default destination of this map or nullptr. */

};

/**
 * \brief Returns the ground property of tiles at the specified point.
 *
 * Only static tiles are considered here (not the dynamic entities).
 * Use get_ground() instead to also take into account dynamic entities that
 * may change the ground, like dynamic tiles and destructible entities.
 *
 * This function assumes that the parameters are correct: for performance
 * reasons, no check is done here.
 *
 * \param layer Layer of the point.
 * \param x X coordinate of the point.
 * \param y Y coordinate of the point.
 * \return The ground of the highest tile at this place.
 */
inline Ground Entities::get_tile_ground(int layer, int x, int y) const {

  return tiles_ground.at(layer)[(y >> 3) * map_width8 + (x >> 3)];
}

/**
 * \brief Returns the camera of the map.
 * \return The camera, or nullptr if there is no camera.
 */
inline const CameraPtr& Entities::get_camera() const {

  return camera;
}

/**
 * \brief Returns all entities of a type.
 * \return All entities of the type.
 */
template<typename T>
std::set<std::shared_ptr<const T>> Entities::get_entities_by_type() const {

  std::set<std::shared_ptr<const T>> result;

  const EntityType type = T::ThisType;
  const auto& it = entities_by_type.find(type);
  if (it == entities_by_type.end()) {
    return result;
  }

  for (const auto& kvp : it->second) {
    for (const ConstEntityPtr& entity : kvp.second) {
      result.insert(std::static_pointer_cast<const T>(entity));
    }
  }
  return result;
}

/**
 * \brief Returns all entities of a type (non-const version).
 * \return All entities of the type.
 */
template<typename T>
std::set<std::shared_ptr<T>> Entities::get_entities_by_type() {

  std::set<std::shared_ptr<T>> result;

  const EntityType type = T::ThisType;
  const auto& it = entities_by_type.find(type);
  if (it == entities_by_type.end()) {
    return result;
  }

  for (const auto& kvp : it->second) {
    for (const EntityPtr& entity : kvp.second) {
      result.insert(std::static_pointer_cast<T>(entity));
    }
  }
  return result;
}

/**
 * \brief Returns all entities of a type on the given layer.
 * \param layer The layer to get entities from.
 * \return All entities of the type on this layer.
 */
template<typename T>
std::set<std::shared_ptr<const T>> Entities::get_entities_by_type(int layer) const {

  std::set<std::shared_ptr<const T>> result;

  const EntityType type = T::ThisType;
  const auto& it = entities_by_type.find(type);
  if (it == entities_by_type.end()) {
    return result;
  }

  const ByLayer<EntitySet>& sets = it->second;
  const auto& layer_it = sets.find(layer);
  if (layer_it == sets.end()) {
    return result;
  }
  for (const EntityPtr& entity : layer_it->second) {
    result.insert(std::static_pointer_cast<const T>(entity));
  }
  return result;
}

/**
 * \brief Returns all entities of a type on the given layer (non-const version).
 * \param layer The layer to get entities from.
 * \return All entities of the type on this layer.
 */
template<typename T>
std::set<std::shared_ptr<T>> Entities::get_entities_by_type(int layer) {

  std::set<std::shared_ptr<T>> result;

  const EntityType type = T::ThisType;
  const auto& it = entities_by_type.find(type);
  if (it == entities_by_type.end()) {
    return result;
  }

  const ByLayer<EntitySet>& sets = it->second;
  const auto& layer_it = sets.find(layer);
  if (layer_it == sets.end()) {
    return result;
  }
  for (const EntityPtr& entity : layer_it->second) {
    result.insert(std::static_pointer_cast<T>(entity));
  }
  return result;
}

}

#endif

