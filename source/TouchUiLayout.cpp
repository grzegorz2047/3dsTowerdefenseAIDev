#include "TouchUiLayout.hpp"

namespace {

constexpr TouchRect kBallista{8, 8, 96, 44};
constexpr TouchRect kMortar{112, 8, 96, 44};
constexpr TouchRect kFrost{216, 8, 96, 44};
constexpr TouchRect kCancel{240, 128, 72, 40};
constexpr TouchRect kPause{8, 128, 72, 40};
constexpr TouchRect kSpeed{88, 128, 72, 40};
constexpr TouchRect kBuild{8, 184, 72, 48};
constexpr TouchRect kUpgrade{88, 184, 72, 48};
constexpr TouchRect kSell{168, 184, 64, 48};
constexpr TouchRect kStart{240, 184, 72, 48};

}  // namespace

TouchUiAction TouchUiLayout::actionAt(std::int16_t x, std::int16_t y) {
    if (x < 0 || y < 0 || x >= kScreenWidth || y >= kScreenHeight) {
        return TouchUiAction::None;
    }
    if (kBallista.contains(x, y)) return TouchUiAction::SelectBallista;
    if (kMortar.contains(x, y)) return TouchUiAction::SelectMortar;
    if (kFrost.contains(x, y)) return TouchUiAction::SelectFrost;
    if (kPause.contains(x, y)) return TouchUiAction::TogglePause;
    if (kSpeed.contains(x, y)) return TouchUiAction::ToggleSpeed;
    if (kCancel.contains(x, y)) return TouchUiAction::Cancel;
    if (kBuild.contains(x, y)) return TouchUiAction::BuildOrSelect;
    if (kUpgrade.contains(x, y)) return TouchUiAction::Upgrade;
    if (kSell.contains(x, y)) return TouchUiAction::Sell;
    if (kStart.contains(x, y)) return TouchUiAction::StartWave;
    return TouchUiAction::None;
}

TouchRect TouchUiLayout::rectFor(TouchUiAction action) {
    switch (action) {
        case TouchUiAction::SelectBallista: return kBallista;
        case TouchUiAction::SelectMortar: return kMortar;
        case TouchUiAction::SelectFrost: return kFrost;
        case TouchUiAction::BuildOrSelect: return kBuild;
        case TouchUiAction::Upgrade: return kUpgrade;
        case TouchUiAction::Sell: return kSell;
        case TouchUiAction::StartWave: return kStart;
        case TouchUiAction::TogglePause: return kPause;
        case TouchUiAction::ToggleSpeed: return kSpeed;
        case TouchUiAction::Cancel: return kCancel;
        case TouchUiAction::None:
        default: return {};
    }
}
