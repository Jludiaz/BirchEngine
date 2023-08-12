#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <bitset>
#include <array>

class Component;
class Entity;

using ComponentID = std::size_t;

/*
The inline keyword tells the compiler to substitute 
the code within the function definition for every 
instance of a function call.
*/

inline ComponentID getComponentTypeID()
/*
* This is a function that returns a unique component identifier (ComponentID) each time it is called.
It uses a static variable lastID to keep track of the last assigned ID and increments it after returning the value.
The inline keyword suggests the compiler to replace the function call with the actual function code to potentially improve performance.
*/
{
	static ComponentID lastID = 0; 
	return lastID++;
}

template <typename T> inline ComponentID getComponentTypeID() noexcept
{
	static ComponentID typeID = getComponentTypeID();
	return typeID;
}

 //We need to tell if an entity has a component attached

constexpr std::size_t maxComponents = 32;

using ComponentBitSet = std::bitset<maxComponents>;
using ComponentArray = std::array<Component*, maxComponents>;

class Component
{
public:
	Entity* entity;

	virtual void init () {}
	virtual void update() {}
	virtual void draw() {}

	virtual ~Component() {}
};

class Entity
{
private:
	bool active = true; //if its false remove 
	std::vector<std::unique_ptr<Component>> components; //holds the components associated with the entity

	ComponentArray componentArray; //an array of pointers to components
	ComponentBitSet componentBitSet; //represents whether an entity has a specific component

public:
	void update() //This function iterates over all components of the entity and calls their update() and draw() functions.
	{
		for (auto& c : components) c->update();
	}
	void draw() 
	{
		for (auto& c : components) c->draw();
	}
	bool isActive() const { return active; }
	void destroy() { active = false; } //find any inactive entities and remove them from the game

	template <typename T> bool hasComponent() const
	{
		return ComponentBitSet(getComponentTypeID<T>); //check if it has component of type t

	}

	template <typename T, typename... TArgs>
	T& addComponent(TArgs&&... mArgs)
	{
		T* c(new T(std::forward<TArgs>(mArgs)...));
		c->entity = this;
		std::unique_ptr<Component> uPtr{ c };
		components.emplace_back(std::move(uPtr));

		componentArray[getComponentTypeID<T>()] = c;
		componentBitSet[getComponentTypeID<T>()] = true;

		c->init();

		return *c;
	}

	template<typename T> T& getComponent() const
	{
		auto ptr(componentArray[getComponentTypeID<T>()]);
		return *static_cast<T*>(ptr);
	}
};

class Manager //This class is responsible for managing entities in the ECS framework.
{
private:
	std::vector<std::unique_ptr<Entity>> entities; //Holds all the entities

public:
	void update() //Updates Entities
	{     
		for (auto& e : entities) e->update();
	}

	void draw()//Draws all entities
	{
		for (auto& e : entities) e->draw();
	} 

	void refresh() //removes any inactive entities
	{
		entities.erase(std::remove_if(std::begin(entities), std::end(entities),
			[](const std::unique_ptr<Entity>& mEntity)
			{
				return !mEntity-> isActive();
			}),
				std::end(entities));
	}

	Entity& addEntity() 
/*This function creates a new entity, adds it to the manager's 
collection, and returns a reference to the newly created entity.*/
	{
		Entity* e = new Entity();
		std::unique_ptr<Entity> uPtr{ e };
		entities.emplace_back(std::move(uPtr));
		return *e;
	}
};