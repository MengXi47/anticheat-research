#include "ue4/items_table.h"

namespace ue4 {

const char *GetItemNameByID(int items_id, int lang) {
  const bool zh = (lang == 1);
  switch (items_id) {
    case 101001:
      return "AKM";
    case 101002:
      return "M16A4";
    case 101003:
      return "SCAR-L";
    case 101004:
      return "M416";
    case 101005:
      return "Groza";
    case 101006:
      return "AUG";
    case 101007:
      return "QBZ";
    case 101008:
      return "M762";
    case 101009:
      return "Mk47";
    case 101010:
      return "G36C";
    case 101012:
      return zh ? "蜜獾" : "Honey Badger";
    case 101100:
      return "FAMAS";
    case 101101:
      return "AN-94";
    case 101102:
      return "ACE32";
    case 102001:
      return "UZI";
    case 102002:
      return "UMP9";
    case 102003:
      return "Vector";
    case 102004:
      return zh ? "湯姆遜" : "Tommy Gun";
    case 102005:
      return zh ? "野牛" : "Bizon";
    case 102007:
      return "MP5K";
    case 102008:
      return "JS9";
    case 102105:
      return "P90";
    case 103001:
      return "Kar98k";
    case 103002:
      return "M24";
    case 103003:
      return "AWM";
    case 103004:
      return "SKS";
    case 103005:
      return "VSS";
    case 103006:
      return "Mini-14";
    case 103007:
      return "Mk14 EBR";
    case 103008:
      return "Win94";
    case 103009:
      return "SLR";
    case 103010:
      return "QBU";
    case 103011:
      return zh ? "莫辛納甘" : "Mosin-Nagant";
    case 103012:
      return "AMR";
    case 103100:
      return "MK12";
    case 103102:
      return "DSR-1";
    case 103103:
      return "M1 Garand";
    case 104001:
      return "S686";
    case 104002:
      return "S1897";
    case 104003:
      return "S12K";
    case 104004:
      return "DBS";
    case 104101:
      return "M1014";
    case 104102:
      return "Neostead 2000";
    case 105001:
      return "M249";
    case 105002:
      return "DP-28";
    case 105010:
      return "MG3";
    case 106001:
      return "P92";
    case 106002:
      return "P1911";
    case 106003:
      return "R1895";
    case 106004:
      return "P18C";
    case 106005:
      return "R45";
    case 106006:
      return zh ? "短管霰彈槍" : "Sawed-Off";
    case 106008:
      return "Vz.61 Skorpion";
    case 106010:
      return "Desert Eagle";
    case 106011:
      return "MP7";
    case 106012:
      return zh ? "焊接工具" : "Welding Tool";
    case 106013:
      return zh ? "電擊槍" : "Taser";
    case 107001:
      return zh ? "十字弩" : "Crossbow";
    case 107007:
      return zh ? "十字弩(無主之地)" : "Crossbow (Borderlands)";
    case 107008:
      return zh ? "獵弓" : "Hunting Bow";
    case 107010:
      return zh ? "煙霧發射器" : "Smoke Launcher";
    case 107011:
      return zh ? "迫擊砲" : "Mortar";
    case 108001:
      return zh ? "砍刀" : "Machete";
    case 108004:
      return zh ? "平底鍋" : "Pan";
    case 108005:
      return zh ? "匕首" : "Dagger";
    case 201001:
      return zh ? "收束器" : "Choke";
    case 201002:
      return zh ? "補償器(衝鋒槍)" : "Compensator (Mid)";
    case 201003:
      return zh ? "補償器(狙擊)" : "Compensator (Sniper)";
    case 201004:
      return zh ? "消焰器(衝鋒槍)" : "Flash Hider (Mid)";
    case 201005:
      return zh ? "消焰器(狙擊)" : "Flash Hider (Sniper)";
    case 201006:
      return zh ? "消音器(衝鋒槍)" : "Suppressor (Mid)";
    case 201007:
      return zh ? "消音器(狙擊)" : "Suppressor (Sniper)";
    case 201009:
      return zh ? "補償器(步槍)" : "Compensator (Large)";
    case 201010:
      return zh ? "消焰器(步槍)" : "Flash Hider (Large)";
    case 201011:
      return zh ? "消音器(步槍)" : "Suppressor (Large)";
    case 201012:
      return zh ? "鴨嘴" : "Duckbill";
    case 201050:
      return zh ? "加長槍管(衝鋒槍)" : "Extended Barrel (Small)";
    case 201051:
      return zh ? "加長槍管(步槍)" : "Extended Barrel (Large)";
    case 201052:
      return zh ? "制退器(狙擊)" : "Muzzle Brake (Sniper)";
    case 201053:
      return zh ? "制退器(步槍)" : "Muzzle Brake (Large)";
    case 201055:
      return zh ? "加長槍管(狙擊)" : "Extended Barrel (Sniper)";
    case 202001:
      return zh ? "斜角前握把" : "Angled Grip";
    case 202002:
      return zh ? "垂直前握把" : "Vertical Grip";
    case 202004:
      return zh ? "輕型前握把" : "Light Grip";
    case 202005:
      return zh ? "半截式握把" : "Half Grip";
    case 202006:
      return zh ? "拇指握把" : "Thumb Grip";
    case 202007:
      return zh ? "激光瞄準器" : "Laser Sight";
    case 202051:
      return zh ? "人體工學握把" : "Ergo Grip";
    case 203001:
      return zh ? "紅點瞄準鏡" : "Red Dot Sight (Holosight)";
    case 203002:
      return zh ? "全息瞄準鏡" : "Red Dot Sight (Reflex)";
    case 203003:
      return zh ? "2倍鏡" : "2x Scope";
    case 203004:
      return zh ? "4倍鏡" : "4x Scope";
    case 203005:
      return zh ? "8倍鏡" : "8x Scope";
    case 203014:
      return zh ? "3倍鏡" : "3x Scope";
    case 203015:
      return zh ? "6倍鏡" : "6x Scope";
    case 203018:
      return zh ? "RMR瞄準鏡" : "RMR Sight";
    case 204004:
      return zh ? "擴容彈匣(衝鋒槍)" : "Extended Mag (SMG)";
    case 204005:
      return zh ? "快速彈匣(衝鋒槍)" : "Quickdraw Mag (SMG)";
    case 204006:
      return zh ? "快速擴容彈匣(衝鋒槍)" : "Extended Quickdraw (SMG)";
    case 204007:
      return zh ? "擴容彈匣(狙擊)" : "Extended Mag (Sniper)";
    case 204008:
      return zh ? "快速彈匣(狙擊)" : "Quickdraw Mag (Sniper)";
    case 204009:
      return zh ? "快速擴容彈匣(狙擊)" : "Extended Quickdraw (Sniper)";
    case 204011:
      return zh ? "擴容彈匣(步槍)" : "Extended Mag (AR)";
    case 204012:
      return zh ? "快速彈匣(步槍)" : "Quickdraw Mag (AR)";
    case 204013:
      return zh ? "快速擴容彈匣(步槍)" : "Extended Quickdraw (AR)";
    case 204014:
      return zh ? "狙擊彈匣" : "Sniper Mag";
    case 204051:
      return zh ? "彈鼓" : "Drum Magazine";
    case 205001:
      return zh ? "UZI槍托" : "UZI Stock";
    case 205002:
      return zh ? "戰術槍托(A型)" : "Tactical Stock (Type A)";
    case 205003:
      return zh ? "托腮版" : "Sniper Stock";
    case 205004:
      return zh ? "弩用槍托(Q型)" : "Crossbow Stock (Type Q)";
    case 205010:
      return zh ? "戰術槍托(H型)" : "Tactical Stock (Type H)";
    case 207001:
      return zh ? "鴨嘴配件(A型)" : "Duckbill Attachment (Type A)";
    case 208001:
      return zh ? "榴彈發射器(A型)" : "Grenade Launcher (Type A)";
    case 209001:
      return zh ? "弩戰術配件" : "Crossbow Tactical";
    case 301001:
      return zh ? "9mm子彈" : "9mm Ammo";
    case 302001:
      return zh ? "7.62mm子彈" : "7.62mm Ammo";
    case 303001:
      return zh ? "5.56mm子彈" : "5.56mm Ammo";
    case 304001:
      return zh ? "12號鉛彈" : "12 Gauge Ammo";
    case 305001:
      return zh ? ".45 ACP子彈" : ".45 ACP Ammo";
    case 306001:
      return zh ? ".300麥格農子彈" : ".300 Magnum Ammo";
    case 307001:
      return zh ? "弩箭" : "Bolt (Crossbow)";
    case 403045:
      return zh ? "吉利服(沙漠)" : "Ghillie Suit (Desert)";
    case 403187:
      return zh ? "吉利服(森林)" : "Ghillie Suit (Forest)";
    case 403989:
      return zh ? "吉利服(雪地)" : "Ghillie Suit (Snow)";
    case 403990:
      return zh ? "吉利服(城市)" : "Ghillie Suit (Urban)";
    case 501001:
      return zh ? "一級背包" : "Backpack (Level 1)";
    case 501004:
      return zh ? "一級背包" : "Backpack (Level 1)";
    case 501005:
      return zh ? "二級背包" : "Backpack (Level 2)";
    case 501002:
      return zh ? "二級背包" : "Backpack (Level 2)";
    case 501006:
      return zh ? "三級背包" : "Backpack (Level 3)";
    case 502001:
      return zh ? "一級頭盔" : "Helmet (Level 1)";
    case 502004:
      return zh ? "一級頭盔" : "Helmet (Level 1)";
    case 502002:
      return zh ? "二級頭盔" : "Helmet (Level 2)";
    case 502005:
      return zh ? "二級頭盔" : "Helmet (Level 2)";
    case 502003:
      return zh ? "三級頭盔" : "Helmet (Level 3)";
    case 503001:
      return zh ? "一級防彈衣" : "Vest (Level 1)";
    case 503004:
      return zh ? "一級防彈衣" : "Vest (Level 1)";
    case 503002:
      return zh ? "二級防彈衣" : "Vest (Level 2)";
    case 503003:
      return zh ? "三級防彈衣" : "Vest (Level 3)";
    case 601001:
      return zh ? "能量飲料" : "Energy Drink";
    case 601002:
      return zh ? "腎上腺素" : "Syringe/Injection";
    case 601003:
      return zh ? "止痛藥" : "Pills/Medication";
    case 601004:
      return zh ? "繃帶" : "Bandage";
    case 601005:
      return zh ? "急救包" : "First Aid Kit";
    case 601006:
      return zh ? "醫療箱" : "Med Kit";
    case 601104:
      return zh ? "夏日烤肉" : "Summer BBQ";
    case 602001:
      return zh ? "震撼彈" : "Stun Grenade";
    case 602002:
      return zh ? "煙霧彈" : "Smoke Grenade";
    case 602003:
      return zh ? "燃燒瓶" : "Molotov Cocktail";
    case 602004:
      return zh ? "手榴彈" : "Frag Grenade";
    case 602005:
      return zh ? "蘋果手雷" : "Apple Grenade";
    case 602123:
      return zh ? "黏彈" : "Sticky Grenade";
    case 602036:
      return zh ? "地刺" : "Spike Strip";
    case 603001:
      return zh ? "汽油桶" : "Gas Can";
    case 10310301:
      return zh ? "M1加蘭德" : "M1 Garand (Ghillie)";
    case 3000324:
      return zh ? "商店代幣" : "Coin";
    case 3000335:
      return zh ? "密室鑰匙" : "Crate Key";
    case 108035:
      return zh ? "隨身治療裝備" : "Survival Equipment";
    case 107005:
      return zh ? "超派鐵拳" : "Rocket Launcher (Iron Fist)";
    case 1702914:
      return zh ? "福利兌換券" : "Welfare Coupon";
    case 601095:
      return zh ? "自救器" : "Self Rescue Kit";
    case 602069:
      return zh ? "傘包" : "Parachute Pack";
    case 44060803:
      return zh ? "燭之翅" : "Candle Wings";
    case 44060802:
      return zh ? "神怒之心" : "Heart of Wrath";
    case 604020:
      return zh ? "腳踏車" : "Bicycle";
    case 308001:
      return zh ? "信號彈" : "Signal Flare";
    case 106007:
      return zh ? "信號槍" : "Flare Gun";
    default:
      return nullptr;
  }
}

ItemCategory GetItemCategory(int items_id) {
  switch (items_id) {
    case 10310301:
      return ItemCategory::Sniper;
    case 108035:
    case 604020:
      return ItemCategory::Consumable;
    case 601095:
    case 3000335:
    case 308001:
    case 106007:
      return ItemCategory::RareLoot;
    case 44060803:
    case 44060802:
      return ItemCategory::Event;
    case 3000324:
    case 1702914:
      return ItemCategory::Misc;
    default:
      break;
  }
  const int hi = items_id / 1000;
  switch (hi) {
    case 101:
      return ItemCategory::Rifle;
    case 102:
      return ItemCategory::SMG;
    case 103:
      return ItemCategory::Sniper;
    case 104:
      return ItemCategory::Shotgun;
    case 105:
      return ItemCategory::LMG;
    case 106:
      return ItemCategory::Pistol;
    case 107:
      return ItemCategory::SpecialWeapon;
    case 108:
      return ItemCategory::Melee;
    case 403:
      return ItemCategory::Ghillie;
    case 601:
      return ItemCategory::Consumable;
    case 602:
      return ItemCategory::Throwable;
    case 603:
      return ItemCategory::Misc;
    default:
      break;
  }
  if (hi >= 201 && hi <= 209) {
    return ItemCategory::Attachment;
  }
  if (hi >= 301 && hi <= 307) {
    return ItemCategory::Ammo;
  }
  if (hi >= 501 && hi <= 503) {
    return ItemCategory::Armor;
  }
  return ItemCategory::Count;
}

const char *VehicleName(int type, int lang) {
  const bool zh = lang != 0;
  switch (type) {
    case 1:
    case 2:
    case 3:
    case 4:
      return zh ? "機車" : "Motorbike";
    case 5:
    case 6:
    case 7:
    case 8:
      return zh ? "轎車" : "Dacia";
    case 9:
    case 10:
    case 11:
    case 41:
    case 68:
      return zh ? "吉普" : "UAZ";
    case 12:
    case 13:
    case 14:
    case 30:
    case 31:
    case 32:
      return zh ? "蹦蹦" : "Buggy";
    case 15:
      return zh ? "快艇" : "Speedboat";
    case 16:
      return zh ? "水上摩托" : "Aquarail";
    case 17:
    case 18:
    case 19:
      return zh ? "巴士" : "MiniBus";
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
      return zh ? "貨卡" : "Pickup";
    case 33:
    case 34:
    case 35:
    case 36:
    case 37:
    case 38:
    case 39:
    case 40:
      return zh ? "沙跑" : "Mirado";
    case 42:
    case 43:
    case 44:
      return "Rony";
    case 45:
      return zh ? "滑板車" : "Scooter";
    case 47:
      return zh ? "直升機" : "Helicopter";
    case 48:
      return zh ? "兩棲車" : "Amphibious";
    case 49:
      return zh ? "嘟嘟車" : "Tukshai";
    case 55:
      return zh ? "龍舟" : "Dragon Boat";
    case 56:
    case 64:
      return "Tesla";
    case 60:
      return zh ? "物資車" : "Loot Truck";
    case 65:
      return "ATV";
    default:
      return zh ? "載具" : "Vehicle";
  }
}

}  // namespace ue4
