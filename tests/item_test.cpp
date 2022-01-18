#include <iostream>
#include <tempo.h>

#include "ui/main_window.h"
#include "core/item_manager.h"


using namespace core;
int main() {
    Tempo::Config config{
        .app_name = "Gestmat",
        .app_title = "Gestion matériel",
    };

    Item::Manager manager;

    auto prop_num = manager.createProperty(Item::Property{
        .name = "N°",
        .mandatory = false,
        .no_edit = false
        });
    auto prop_type = manager.createProperty(Item::Property{
        .name = "Type",
        .mandatory = true,
        .no_edit = false
        });
    auto prop_side = manager.createProperty(Item::Property{
        .name = "Side",
        .mandatory = true,
        .no_edit = false,
        .select = {"left", "right"},
        });

    std::cout << prop_num << " " << prop_type << " " << prop_side << std::endl;

    assert(!manager.isRetired(prop_side).value());
    assert(manager.retireProperty(prop_side));
    assert(!manager.retireProperty(prop_side));
    assert(manager.isRetired(prop_side).value());
    assert(manager.unretireProperty(prop_side));
    manager.retireProperty(prop_num);

    auto cat_fr = manager.createCategory(Item::Category{
        .name = "Fauteuil roulants",
        .properties = {prop_num, prop_side, prop_type},
        });

    assert(cat_fr == 3);
    assert(!manager.isRetired(prop_num).value());

    auto cat_acc = manager.createCategory(Item::Category{
        .name = "Accessoires de fauteuil roulants",
        .properties = {prop_num, prop_type},
        });

    auto item_bad = manager.createItem(Item::Item{
        .category = prop_side,
        });
    assert(item_bad == -1);


    auto item1 = manager.createItem(Item::Item{
        .category = cat_fr,
        .property_values = {{prop_side, "left"}, {prop_num, "1"}},
        });
    assert(item1 == 6);


    auto item2 = manager.createItem(Item::Item{
        .category = cat_acc,
        .property_values = {{prop_side, "left"}, {prop_num, "1"}},
        });

    manager.retireItem(item2);
    assert(manager.isRetired(item2));

    auto loan1 = manager.newLoan(
        item1,
        Item::Note{ .content = "" },
        getCurrentDate(),
        Item::Person{
            .name = "Jean",
            .surname = "François",
        }
    ).value();
    Item::Loan_ptr loan = manager.getLoan(loan1).value();

    assert(manager.getPerson(loan->person).value()->name == "Jean");

    auto loan2 = manager.newLoan(
        cat_acc,
        Item::Note{ .content = "" },
        getCurrentDate(),
        loan->person
    );

    assert(!loan2.has_value());

    auto loan3 = manager.newLoan(
        item2,
        Item::Note{ .content = "" },
        getCurrentDate(),
        loan->person
    ).value();

    assert(manager.getAllLoans().size() == 2);
    manager.retireLoan(loan3);
    assert(manager.isRetired(loan3).value());


    // MainApp* app = new MainApp();
    // Tempo::Run(app, config);

    return 0;
}