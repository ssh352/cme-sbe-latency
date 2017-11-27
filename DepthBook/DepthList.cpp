//
// Created by antoine on 22/11/17.
//

#include "DepthList.h"
#include <numeric>

DepthList::DepthList(bool isbid) : isbid_(isbid) {
  levels_.reserve(MAX_BOOK_SIZE + 1); // +1 allows for add to occur at level 1
                                      // which temporarily pushes book size ==
                                      // max + 1
}

void DepthList::Add(int level, float price, int quantity) {
  if (level - 1 <= levels_.size()) {
    levels_.insert(levels_.begin() + level - 1, {price, quantity});

    if (levels_.size() > MAX_BOOK_SIZE)
      levels_.erase(levels_.begin() + MAX_BOOK_SIZE);
  } else
    std::cerr << "Add out of range." << '\n';
}

void DepthList::Update(int level, float price, int quantity) {
  if (level - 1 < levels_.size()) {
    auto &price_level = levels_[level - 1];
    if (price_level.price == price)
      price_level.quantity = quantity;
    else
      std::cerr << "Price mismatch on update request." << '\n';
  } else {

    std::cerr << "Update bid level out of range." << '\n';
  }
}

void DepthList::Delete(int level, float price) {
  if (level - 1 < levels_.size()) {

    auto it = levels_.begin() + level - 1;
    if (it->price == price)
      levels_.erase(it);
    else
      std::cerr << "Price mismatch on delete request." << '\n';
  } else {
    std::cerr << "Delete level out of range." << '\n';
  }
}

void DepthList::DeleteFrom(int level) {
  if (level - 1 < levels_.size())
    levels_.erase(levels_.begin() + level - 1, levels_.end());
  else
    std::cerr << "Delete bid from level out of range." << '\n';
}

void DepthList::DeleteThru() { levels_.clear(); }

void DepthList::Clear() { levels_.clear(); }

std::string DepthList::ToString() {

  std::string prices = std::accumulate(
      std::next(levels_.begin()), levels_.end(),
      std::to_string(levels_[0].price), // start with first element
      [](std::string a, PriceLevel b) {
        return a + ',' + std::to_string(b.price);
      });

  std::string quantities = std::accumulate(
      std::next(levels_.begin()), levels_.end(),
      std::to_string(levels_[0].quantity), // start with first element
      [](std::string a, PriceLevel b) {
        return a + ',' + std::to_string(b.quantity);
      });

  return "[" + prices + "],[" + quantities + "]";
}
/*
struct object
{
  int a;
  int b;
};

constexpr char ListSize = 12;
std::array<object, ListSize> list;
std::array<std::pair<std::string, std::string>, ListSize> strings;
std::transform(std::begin(list), std::end(list), std::begin(strings), [](const
object& input)
{
return { std::to_string(input.a),  std::to_string(input.b) };
});
*/

std::ostream &operator<<(std::ostream &os, const DepthList &list) {
  if (list.isbid_) {
    for (auto it = list.levels_.begin(); it != list.levels_.end(); ++it) {
      os << it->quantity << "\t\t" << it->price << '\n';
    }
  } else {
    for (auto it = list.levels_.rbegin(); it != list.levels_.rend(); ++it) {
      os << "\t\t" << it->price << "\t\t" << it->quantity << '\n';
    }
  }
  return os;
}
