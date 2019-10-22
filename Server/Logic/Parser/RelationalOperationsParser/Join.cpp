#include "Join.h"
#include "../../../Engine/Engine.h"

Table Join::makeJoin(const Table& table1, const Table& table2,
                     Expression* on_expr, std::unique_ptr<exc::Exception>& e) {
    if (e) {
        return Table();
    }

    /*   это все делается перед джоином
     *
     * auto child1 = root->childs()[0];
        auto child2 = root->childs()[1];

        auto table_id1 = (child1->getAlias().empty()) ? (child1->getName())
                                                      : (child1->getAlias());
        auto table_id2 = (child2->getAlias().empty()) ? (child2->getName())
                                                      : (child2->getAlias());

        auto table1 = Engine::show(table_id1, e);
        if (e) {
            return Table();
        }
        auto table2 = Engine::show(table_id2, e);
        if (e) {
            return Table();
        }*/

    auto siz1 = table1.getColumns().size();
    auto siz2 = table2.getColumns().size();

    if (siz1 * siz2 > siz1 + siz2) {
        // hash
    } else {
        // square
    }
}
