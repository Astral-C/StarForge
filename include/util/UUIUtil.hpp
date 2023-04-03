#pragma once

#include <magic_enum.hpp>
#include <imgui.h>
#include <algorithm>
#include <string>

namespace UIUtil {
	// Renders a combobox for the given enum.
	template<typename T>
	bool RenderComboEnum(std::string name, T& current_value)
	{
		static_assert(std::is_enum_v<T>, "T must be an enum!");

		bool changed = false;
		auto origName = magic_enum::enum_name(current_value);
		std::string curName{ origName.begin(), origName.end() };
		std::replace(curName.begin(), curName.end(), '_', ' ');

		// Combobox start
		if (ImGui::BeginCombo(name.c_str(), curName.c_str()))
		{
			// Iterating the possible enum values...
			for (auto [enum_value, enum_name] : magic_enum::enum_entries<T>())
			{
				std::string displayName{ enum_name.begin(), enum_name.end() };
				std::replace(displayName.begin(), displayName.end(), '_', ' ');

				// ImGui ID stack is now at <previous value>##<enum value>
				ImGui::PushID(static_cast<int>(enum_value));

				// Render the combobox item for this enum value
				bool is_selected = (current_value == enum_value);
				if (ImGui::Selectable(displayName.c_str(), is_selected))
				{
					current_value = enum_value;
					changed = true;
				}

				// Set initial focus when opening the combo
				if (is_selected)
					ImGui::SetItemDefaultFocus();

				// ImGui ID stack returns to <previous value>
				ImGui::PopID();
			}

			// End combobox
			ImGui::EndCombo();
		}

		return changed;
	}
}
