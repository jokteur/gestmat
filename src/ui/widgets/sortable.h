#pragma once

#include <algorithm>
#include <vector>
#include <string>

#include <tempo.h>

#include "core/item_manager.h"

template<class T, class U>
struct SortableRow {
    int num_cols = 0;
    U infos;
    std::map<ImGuiID, T> items;
    std::map<ImGuiID, std::string> strings;

    static const ImGuiTableSortSpecs* s_current_sort_specs;

    static int CompareWithSortSpecs(const void* lhs, const void* rhs) {
        SortableRow<T, U>* a = (SortableRow<T, U>*)lhs;
        SortableRow<T, U>* b = (SortableRow<T, U>*)rhs;

        for (int n = 0; n < s_current_sort_specs->SpecsCount; n++) {
            const ImGuiTableColumnSortSpecs* sort_spec = &s_current_sort_specs->Specs[n];
            int delta = 0;

            delta = a->strings[sort_spec->ColumnUserID] < b->strings[sort_spec->ColumnUserID];

            if (delta)
                return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? +1 : -1;
            if (!delta)
                return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? -1 : +1;
        }
    }
};

using namespace core;
const ImGuiTableSortSpecs* SortableRow<Item::PropertyID, Item::Item_ptr>::s_current_sort_specs = NULL;
