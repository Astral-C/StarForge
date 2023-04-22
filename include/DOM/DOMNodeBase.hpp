#pragma once
#include "GenUtil.hpp"

#include <vector>
#include <memory>
#include <string>
#include <functional>

enum EGameType {
    SMG1,
    SMG2
};

enum EDOMNodeState
{
	EDOMNodeState_Initialized = 0x01,
	EDOMNodeState_Rendered = 0x02,
	EDOMNodeState_Selected = 0x04
};

enum class EDOMNodeType
{
	Base,
	Serializable,
    Galaxy,
    Scenario,
	Zone,
	ZoneLayer,

	// Entities
	Entity,
    Object
};

// Base class for all DOM (Document Object Model) nodes.
class SDOMNodeBase : public std::enable_shared_from_this<SDOMNodeBase>
{
	uint32_t mNodeState { 0 };

protected:
	EDOMNodeType mType;
	std::string mName;

	template<typename T>
	void GatherChildrenOfType(std::vector<std::shared_ptr<T>>& list, EDOMNodeType type)
	{
		for (std::shared_ptr<SDOMNodeBase> child : Children)
		{
			if (child->IsNodeType(type))
				list.push_back(std::static_pointer_cast<T>(child));

			child->GatherChildrenOfType(list, type);
		}
	}

	template<typename T, typename U>
	void GatherChildrenOfType(std::vector<std::shared_ptr<T>>& list, EDOMNodeType type, U predicate)
	{
		for (std::shared_ptr<SDOMNodeBase> child : Children)
		{
			if (child->IsNodeType(type))
			{
				auto typed = std::static_pointer_cast<T>(child);

				if (predicate(typed))
					list.push_back(typed);
			}

			child->GatherChildrenOfType(list, type, predicate);
		}
	}

public:
	SDOMNodeBase(std::string name) { mName = name; SetIsSelected(false); SetIsRendered(true); SetIsInitialized(false); }

	std::weak_ptr<SDOMNodeBase> Parent;
	std::vector<std::shared_ptr<SDOMNodeBase>> Children;

    virtual void RenderHeirarchyUI(std::shared_ptr<SDOMNodeBase>& selected) = 0;
    virtual void RenderDetailsUI() = 0;

	virtual std::string GetName() { return mName; }

/*=== Node state ===*/
	bool GetIsInitialized() { return (mNodeState & EDOMNodeState_Initialized) != 0; }
	bool GetIsRendered() { return (mNodeState & EDOMNodeState_Rendered) != 0; }
	bool GetIsSelected() { return (mNodeState & EDOMNodeState_Selected) != 0; }

	void SetIsInitialized(bool initialized)
	{
		if (initialized)
			mNodeState |= EDOMNodeState_Initialized;
		else
			mNodeState &= ~EDOMNodeState_Initialized;
	}

	void SetIsRendered(bool rendered)
	{
		if (rendered)
			mNodeState |= EDOMNodeState_Rendered;
		else
			mNodeState &= ~EDOMNodeState_Rendered;
	}

	void SetIsSelected(bool selected)
	{
		if (selected)
			mNodeState |= EDOMNodeState_Selected;
		else
			mNodeState &= ~EDOMNodeState_Selected;
	}

/*=== Type operations ===*/
	virtual const char* GetNodeTypeString() { return "DOM_NODE_BASE"; }

	// Returns the underlying type of this node.
	virtual EDOMNodeType GetNodeType() const { return mType; }

	// Returns whether this node is of the given type, or derives from a node of that type.
	virtual bool IsNodeType(EDOMNodeType type) const
	{
		return mType == type;
	}

	// Returns a shared_ptr representing this node of the given type, if possible;
	// returns nullptr if this node cannot be cast to the given type.
	template<typename T>
	std::shared_ptr<T> GetSharedPtr(EDOMNodeType type)
	{
		if (IsNodeType(type))
			return std::static_pointer_cast<T>(shared_from_this());

		return nullptr;
	}

	// Adds the given new child to this node's Children collection, and sets
	// the child's parent to a shared_ptr of this node.
	void AddChild(std::shared_ptr<SDOMNodeBase> new_child)
	{
		Children.push_back(new_child);

		if (!new_child->Parent.expired())
		{
			auto parentLocked = new_child->Parent.lock();
			for (auto it = parentLocked->Children.begin(); it != parentLocked->Children.end(); ++it)
			{
				if (*it == new_child)
				{
					parentLocked->Children.erase(it);
					break;
				}
			}
		}

		new_child->Parent = shared_from_this();
	}

	void AddChildAtIndex(std::shared_ptr<SDOMNodeBase> new_child, uint32_t index)
	{
		if (index == -1)
		{
			AddChild(new_child);
			return;
		}

		Children.insert(Children.begin() + index, new_child);

		if (!new_child->Parent.expired())
		{
			auto parentLocked = new_child->Parent.lock();
			for (auto it = parentLocked->Children.begin(); it != parentLocked->Children.end(); ++it)
			{
				if (*it == new_child)
				{
					parentLocked->Children.erase(it);
					break;
				}
			}
		}

		new_child->Parent = shared_from_this();
	}

	void RemoveChild(std::shared_ptr<SDOMNodeBase> child)
	{
		ptrdiff_t index = SGenUtility::VectorIndexOf(Children, child);
		if (index == -1)
			return;

		child->Children.clear();
		Children.erase(Children.begin() + index);
	}

	// Returns the ancestor of this node of the given type, recursing up the hierarchy;
	// returns nullptr if no parent of the requested type is found.
	template<typename T>
	std::weak_ptr<T> GetParentOfType(EDOMNodeType type)
	{
		// On the off chance that the parent is invalid, don't try to do anything.
		if (Parent.expired())
			return std::weak_ptr<T>();

		// Grab a temporary shared_ptr for the parent.
		auto parentShared = Parent.lock();

		if (parentShared->IsNodeType(type))
			return std::static_pointer_cast<T>(parentShared);

		return parentShared->GetParentOfType<T>(type);
	}

	// Returns a collection of children of this node of the requested type, recursing down the hierarchy.
	template<typename T>
	std::vector<std::shared_ptr<T>> GetChildrenOfType(EDOMNodeType type)
	{
		std::vector<std::shared_ptr<T>> matchingNodes;

		GatherChildrenOfType(matchingNodes, type);

		return matchingNodes;
	}

	template<typename T>
	void ForEachChildOfType(EDOMNodeType type, std::function<void(std::shared_ptr<T>)> func){
		for(auto& node : Children){
			if(node->IsNodeType(type)) func(std::static_pointer_cast<T>(node));
		}
	}

	template<typename T, typename U>
	std::vector<std::shared_ptr<T>> GetChildrenOfType(EDOMNodeType type, U predicate)
	{
		std::vector<std::shared_ptr<T>> matchingNodes;

		GatherChildrenOfType(matchingNodes, type, predicate);

		return matchingNodes;
	}
};