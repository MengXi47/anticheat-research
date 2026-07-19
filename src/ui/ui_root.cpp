#include "ui/ui_root.h"

#include "ui/esp.h"
#include "ui/fab.h"
#include "ui/menu.h"

namespace ui {

void Draw() {
  esp::Draw();
  fab::Draw();
  menu::Draw();
}

} // namespace ui
