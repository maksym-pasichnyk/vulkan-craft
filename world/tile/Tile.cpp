#include "Tile.hpp"

#include "AcaciaButtonTile.hpp"
#include "AcaciaDoorTile.hpp"
#include "AcaciaFenceGateTile.hpp"
#include "AcaciaPressurePlateTile.hpp"
#include "AcaciaStairsTile.hpp"
#include "AcaciaStandingSignTile.hpp"
#include "AcaciaTrapdoorTile.hpp"
#include "AcaciaWallSignTile.hpp"
#include "ActivatorRailTile.hpp"
#include "AirTile.hpp"
#include "AllowTile.hpp"
#include "AndesiteStairsTile.hpp"
#include "AnvilTile.hpp"
#include "BambooTile.hpp"
#include "BambooSaplingTile.hpp"
#include "BarrelTile.hpp"
#include "BarrierTile.hpp"
#include "BeaconTile.hpp"
#include "BedTile.hpp"
#include "BedrockTile.hpp"
#include "BeetrootTile.hpp"
#include "BellTile.hpp"
#include "BirchButtonTile.hpp"
#include "BirchDoorTile.hpp"
#include "BirchFenceGateTile.hpp"
#include "BirchPressurePlateTile.hpp"
#include "BirchStairsTile.hpp"
#include "BirchStandingSignTile.hpp"
#include "BirchTrapdoorTile.hpp"
#include "BirchWallSignTile.hpp"
#include "BlackGlazedTerracottaTile.hpp"
#include "BlastFurnaceTile.hpp"
#include "BlueGlazedTerracottaTile.hpp"
#include "BlueIceTile.hpp"
#include "BoneBlockTile.hpp"
#include "BookshelfTile.hpp"
#include "BorderBlockTile.hpp"
#include "BrewingStandTile.hpp"
#include "BrickBlockTile.hpp"
#include "BrickStairsTile.hpp"
#include "BrownGlazedTerracottaTile.hpp"
#include "BrownMushroomTile.hpp"
#include "BrownMushroomBlockTile.hpp"
#include "BubbleColumnTile.hpp"
#include "CactusTile.hpp"
#include "CakeTile.hpp"
#include "CameraTile.hpp"
#include "CampfireTile.hpp"
#include "CarpetTile.hpp"
#include "CarrotsTile.hpp"
#include "CartographyTableTile.hpp"
#include "CarvedPumpkinTile.hpp"
#include "CauldronTile.hpp"
#include "ChainCommandBlockTile.hpp"
#include "ChestTile.hpp"
#include "ChorusFlowerTile.hpp"
#include "ChorusPlantTile.hpp"
#include "ClayTile.hpp"
#include "CoalBlockTile.hpp"
#include "CoalOreTile.hpp"
#include "CobblestoneTile.hpp"
#include "CobblestoneWallTile.hpp"
#include "CocoaTile.hpp"
#include "CommandBlockTile.hpp"
#include "ComposterTile.hpp"
#include "ConcreteTile.hpp"
#include "ConcretepowderTile.hpp"
#include "ConduitTile.hpp"
#include "CoralTile.hpp"
#include "CoralBlockTile.hpp"
#include "CoralFanTile.hpp"
#include "CoralFanDeadTile.hpp"
#include "CoralFanHangTile.hpp"
#include "CoralFanHang2Tile.hpp"
#include "CoralFanHang3Tile.hpp"
#include "CraftingTableTile.hpp"
#include "CyanGlazedTerracottaTile.hpp"
#include "DarkOakButtonTile.hpp"
#include "DarkOakDoorTile.hpp"
#include "DarkOakFenceGateTile.hpp"
#include "DarkOakPressurePlateTile.hpp"
#include "DarkOakStairsTile.hpp"
#include "DarkOakTrapdoorTile.hpp"
#include "DarkPrismarineStairsTile.hpp"
#include "DarkoakStandingSignTile.hpp"
#include "DarkoakWallSignTile.hpp"
#include "DaylightDetectorTile.hpp"
#include "DaylightDetectorInvertedTile.hpp"
#include "DeadbushTile.hpp"
#include "DenyTile.hpp"
#include "DetectorRailTile.hpp"
#include "DiamondBlockTile.hpp"
#include "DiamondOreTile.hpp"
#include "DioriteStairsTile.hpp"
#include "DirtTile.hpp"
#include "DispenserTile.hpp"
#include "DoublePlantTile.hpp"
#include "DoubleStoneSlabTile.hpp"
#include "DoubleStoneSlab2Tile.hpp"
#include "DoubleStoneSlab3Tile.hpp"
#include "DoubleStoneSlab4Tile.hpp"
#include "DoubleWoodenSlabTile.hpp"
#include "DragonEggTile.hpp"
#include "DriedKelpBlockTile.hpp"
#include "DropperTile.hpp"
#include "EmeraldBlockTile.hpp"
#include "EmeraldOreTile.hpp"
#include "EnchantingTableTile.hpp"
#include "EndBrickStairsTile.hpp"
#include "EndBricksTile.hpp"
#include "EndGatewayTile.hpp"
#include "EndPortalTile.hpp"
#include "EndPortalFrameTile.hpp"
#include "EndRodTile.hpp"
#include "EndStoneTile.hpp"
#include "EnderChestTile.hpp"
#include "FarmlandTile.hpp"
#include "FenceTile.hpp"
#include "FenceGateTile.hpp"
#include "FireTile.hpp"
#include "FletchingTableTile.hpp"
#include "FlowerPotTile.hpp"
#include "FlowingLavaTile.hpp"
#include "FlowingWaterTile.hpp"
#include "FormatVersionTile.hpp"
#include "FrameTile.hpp"
#include "FrostedIceTile.hpp"
#include "FurnaceTile.hpp"
#include "GlassTile.hpp"
#include "GlassPaneTile.hpp"
#include "GlowingobsidianTile.hpp"
#include "GlowstoneTile.hpp"
#include "GoldBlockTile.hpp"
#include "GoldOreTile.hpp"
#include "GoldenRailTile.hpp"
#include "GraniteStairsTile.hpp"
#include "GrassTile.hpp"
#include "GrassPathTile.hpp"
#include "GravelTile.hpp"
#include "GrayGlazedTerracottaTile.hpp"
#include "GreenGlazedTerracottaTile.hpp"
#include "GrindstoneTile.hpp"
#include "HardenedClayTile.hpp"
#include "HayBlockTile.hpp"
#include "HeavyWeightedPressurePlateTile.hpp"
#include "HopperTile.hpp"
#include "IceTile.hpp"
#include "InfoUpdateTile.hpp"
#include "InfoUpdate2Tile.hpp"
#include "InvisiblebedrockTile.hpp"
#include "IronBarsTile.hpp"
#include "IronBlockTile.hpp"
#include "IronDoorTile.hpp"
#include "IronOreTile.hpp"
#include "IronTrapdoorTile.hpp"
#include "JigsawTile.hpp"
#include "JukeboxTile.hpp"
#include "JungleButtonTile.hpp"
#include "JungleDoorTile.hpp"
#include "JungleFenceGateTile.hpp"
#include "JunglePressurePlateTile.hpp"
#include "JungleStairsTile.hpp"
#include "JungleStandingSignTile.hpp"
#include "JungleTrapdoorTile.hpp"
#include "JungleWallSignTile.hpp"
#include "KelpTile.hpp"
#include "LadderTile.hpp"
#include "LanternTile.hpp"
#include "LapisBlockTile.hpp"
#include "LapisOreTile.hpp"
#include "LavaTile.hpp"
#include "LavaCauldronTile.hpp"
#include "LeavesTile.hpp"
#include "Leaves2Tile.hpp"
#include "LecternTile.hpp"
#include "LeverTile.hpp"
#include "LightBlockTile.hpp"
#include "LightBlueGlazedTerracottaTile.hpp"
#include "LightWeightedPressurePlateTile.hpp"
#include "LimeGlazedTerracottaTile.hpp"
#include "LitBlastFurnaceTile.hpp"
#include "LitFurnaceTile.hpp"
#include "LitPumpkinTile.hpp"
#include "LitRedstoneLampTile.hpp"
#include "LitRedstoneOreTile.hpp"
#include "LitSmokerTile.hpp"
#include "LogTile.hpp"
#include "Log2Tile.hpp"
#include "LoomTile.hpp"
#include "MagentaGlazedTerracottaTile.hpp"
#include "MagmaTile.hpp"
#include "MelonBlockTile.hpp"
#include "MelonStemTile.hpp"
#include "MobSpawnerTile.hpp"
#include "MonsterEggTile.hpp"
#include "MossyCobblestoneTile.hpp"
#include "MossyCobblestoneStairsTile.hpp"
#include "MossyStoneBrickStairsTile.hpp"
#include "MovingblockTile.hpp"
#include "MyceliumTile.hpp"
#include "NetherBrickTile.hpp"
#include "NetherBrickFenceTile.hpp"
#include "NetherBrickStairsTile.hpp"
#include "NetherWartTile.hpp"
#include "NetherWartBlockTile.hpp"
#include "NetherrackTile.hpp"
#include "NetherreactorTile.hpp"
#include "NormalStoneStairsTile.hpp"
#include "NoteblockTile.hpp"
#include "OakStairsTile.hpp"
#include "ObserverTile.hpp"
#include "ObsidianTile.hpp"
#include "OrangeGlazedTerracottaTile.hpp"
#include "PackedIceTile.hpp"
#include "PinkGlazedTerracottaTile.hpp"
#include "PistonTile.hpp"
#include "PistonarmcollisionTile.hpp"
#include "PlanksTile.hpp"
#include "PodzolTile.hpp"
#include "PolishedAndesiteStairsTile.hpp"
#include "PolishedDioriteStairsTile.hpp"
#include "PolishedGraniteStairsTile.hpp"
#include "PortalTile.hpp"
#include "PotatoesTile.hpp"
#include "PoweredComparatorTile.hpp"
#include "PoweredRepeaterTile.hpp"
#include "PrismarineTile.hpp"
#include "PrismarineBricksStairsTile.hpp"
#include "PrismarineStairsTile.hpp"
#include "PumpkinTile.hpp"
#include "PumpkinStemTile.hpp"
#include "PurpleGlazedTerracottaTile.hpp"
#include "PurpurBlockTile.hpp"
#include "PurpurStairsTile.hpp"
#include "QuartzBlockTile.hpp"
#include "QuartzOreTile.hpp"
#include "QuartzStairsTile.hpp"
#include "RailTile.hpp"
#include "RedFlowerTile.hpp"
#include "RedGlazedTerracottaTile.hpp"
#include "RedMushroomTile.hpp"
#include "RedMushroomBlockTile.hpp"
#include "RedNetherBrickTile.hpp"
#include "RedNetherBrickStairsTile.hpp"
#include "RedSandstoneTile.hpp"
#include "RedSandstoneStairsTile.hpp"
#include "RedstoneBlockTile.hpp"
#include "RedstoneLampTile.hpp"
#include "RedstoneOreTile.hpp"
#include "RedstoneTorchTile.hpp"
#include "RedstoneWireTile.hpp"
#include "ReedsTile.hpp"
#include "RepeatingCommandBlockTile.hpp"
#include "Reserved6Tile.hpp"
#include "SandTile.hpp"
#include "SandstoneTile.hpp"
#include "SandstoneStairsTile.hpp"
#include "SaplingTile.hpp"
#include "ScaffoldingTile.hpp"
#include "SealanternTile.hpp"
#include "SeaPickleTile.hpp"
#include "SeagrassTile.hpp"
#include "ShulkerBoxTile.hpp"
#include "SilverGlazedTerracottaTile.hpp"
#include "SkullTile.hpp"
#include "SlimeTile.hpp"
#include "SmithingTableTile.hpp"
#include "SmokerTile.hpp"
#include "SmoothQuartzStairsTile.hpp"
#include "SmoothRedSandstoneStairsTile.hpp"
#include "SmoothSandstoneStairsTile.hpp"
#include "SmoothStoneTile.hpp"
#include "SnowTile.hpp"
#include "SnowLayerTile.hpp"
#include "SoulSandTile.hpp"
#include "SpongeTile.hpp"
#include "SpruceButtonTile.hpp"
#include "SpruceDoorTile.hpp"
#include "SpruceFenceGateTile.hpp"
#include "SprucePressurePlateTile.hpp"
#include "SpruceStairsTile.hpp"
#include "SpruceStandingSignTile.hpp"
#include "SpruceTrapdoorTile.hpp"
#include "SpruceWallSignTile.hpp"
#include "StainedGlassTile.hpp"
#include "StainedGlassPaneTile.hpp"
#include "StainedHardenedClayTile.hpp"
#include "StandingBannerTile.hpp"
#include "StandingSignTile.hpp"
#include "StickypistonarmcollisionTile.hpp"
#include "StickyPistonTile.hpp"
#include "StoneTile.hpp"
#include "StoneBrickStairsTile.hpp"
#include "StoneButtonTile.hpp"
#include "StonePressurePlateTile.hpp"
#include "StoneSlabTile.hpp"
#include "StoneSlab2Tile.hpp"
#include "StoneSlab3Tile.hpp"
#include "StoneSlab4Tile.hpp"
#include "StoneStairsTile.hpp"
#include "StonebrickTile.hpp"
#include "StonecutterTile.hpp"
#include "StonecutterBlockTile.hpp"
#include "StrippedAcaciaLogTile.hpp"
#include "StrippedBirchLogTile.hpp"
#include "StrippedDarkOakLogTile.hpp"
#include "StrippedJungleLogTile.hpp"
#include "StrippedOakLogTile.hpp"
#include "StrippedSpruceLogTile.hpp"
#include "StructureBlockTile.hpp"
#include "StructureVoidTile.hpp"
#include "SweetBerryBushTile.hpp"
#include "TallgrassTile.hpp"
#include "TntTile.hpp"
#include "TorchTile.hpp"
#include "TrapdoorTile.hpp"
#include "TrappedChestTile.hpp"
#include "TripwireTile.hpp"
#include "TripwireHookTile.hpp"
#include "TurtleEggTile.hpp"
#include "UndyedShulkerBoxTile.hpp"
#include "UnlitRedstoneTorchTile.hpp"
#include "UnpoweredComparatorTile.hpp"
#include "UnpoweredRepeaterTile.hpp"
#include "VineTile.hpp"
#include "WallBannerTile.hpp"
#include "WallSignTile.hpp"
#include "WaterTile.hpp"
#include "WaterlilyTile.hpp"
#include "WebTile.hpp"
#include "WheatTile.hpp"
#include "WhiteGlazedTerracottaTile.hpp"
#include "WitherRoseTile.hpp"
#include "WoodTile.hpp"
#include "WoodenButtonTile.hpp"
#include "WoodenDoorTile.hpp"
#include "WoodenPressurePlateTile.hpp"
#include "WoodenSlabTile.hpp"
#include "WoolTile.hpp"
#include "YellowFlowerTile.hpp"
#include "YellowGlazedTerracottaTile.hpp"

Tile* Tile::acacia_button;
Tile* Tile::acacia_door;
Tile* Tile::acacia_fence_gate;
Tile* Tile::acacia_pressure_plate;
Tile* Tile::acacia_stairs;
Tile* Tile::acacia_standing_sign;
Tile* Tile::acacia_trapdoor;
Tile* Tile::acacia_wall_sign;
Tile* Tile::activator_rail;
Tile* Tile::air;
Tile* Tile::allow;
Tile* Tile::andesite_stairs;
Tile* Tile::anvil;
Tile* Tile::bamboo;
Tile* Tile::bamboo_sapling;
Tile* Tile::barrel;
Tile* Tile::barrier;
Tile* Tile::beacon;
Tile* Tile::bed;
Tile* Tile::bedrock;
Tile* Tile::beetroot;
Tile* Tile::bell;
Tile* Tile::birch_button;
Tile* Tile::birch_door;
Tile* Tile::birch_fence_gate;
Tile* Tile::birch_pressure_plate;
Tile* Tile::birch_stairs;
Tile* Tile::birch_standing_sign;
Tile* Tile::birch_trapdoor;
Tile* Tile::birch_wall_sign;
Tile* Tile::black_glazed_terracotta;
Tile* Tile::blast_furnace;
Tile* Tile::blue_glazed_terracotta;
Tile* Tile::blue_ice;
Tile* Tile::bone_block;
Tile* Tile::bookshelf;
Tile* Tile::border_block;
Tile* Tile::brewing_stand;
Tile* Tile::brick_block;
Tile* Tile::brick_stairs;
Tile* Tile::brown_glazed_terracotta;
Tile* Tile::brown_mushroom;
Tile* Tile::brown_mushroom_block;
Tile* Tile::bubble_column;
Tile* Tile::cactus;
Tile* Tile::cake;
Tile* Tile::camera;
Tile* Tile::campfire;
Tile* Tile::carpet;
Tile* Tile::carrots;
Tile* Tile::cartography_table;
Tile* Tile::carved_pumpkin;
Tile* Tile::cauldron;
Tile* Tile::chain_command_block;
Tile* Tile::chest;
Tile* Tile::chorus_flower;
Tile* Tile::chorus_plant;
Tile* Tile::clay;
Tile* Tile::coal_block;
Tile* Tile::coal_ore;
Tile* Tile::cobblestone;
Tile* Tile::cobblestone_wall;
Tile* Tile::cocoa;
Tile* Tile::command_block;
Tile* Tile::composter;
Tile* Tile::concrete;
Tile* Tile::concretePowder;
Tile* Tile::conduit;
Tile* Tile::coral;
Tile* Tile::coral_block;
Tile* Tile::coral_fan;
Tile* Tile::coral_fan_dead;
Tile* Tile::coral_fan_hang;
Tile* Tile::coral_fan_hang2;
Tile* Tile::coral_fan_hang3;
Tile* Tile::crafting_table;
Tile* Tile::cyan_glazed_terracotta;
Tile* Tile::dark_oak_button;
Tile* Tile::dark_oak_door;
Tile* Tile::dark_oak_fence_gate;
Tile* Tile::dark_oak_pressure_plate;
Tile* Tile::dark_oak_stairs;
Tile* Tile::dark_oak_trapdoor;
Tile* Tile::dark_prismarine_stairs;
Tile* Tile::darkoak_standing_sign;
Tile* Tile::darkoak_wall_sign;
Tile* Tile::daylight_detector;
Tile* Tile::daylight_detector_inverted;
Tile* Tile::deadbush;
Tile* Tile::deny;
Tile* Tile::detector_rail;
Tile* Tile::diamond_block;
Tile* Tile::diamond_ore;
Tile* Tile::diorite_stairs;
Tile* Tile::dirt;
Tile* Tile::dispenser;
Tile* Tile::double_plant;
Tile* Tile::double_stone_slab;
Tile* Tile::double_stone_slab2;
Tile* Tile::double_stone_slab3;
Tile* Tile::double_stone_slab4;
Tile* Tile::double_wooden_slab;
Tile* Tile::dragon_egg;
Tile* Tile::dried_kelp_block;
Tile* Tile::dropper;
Tile* Tile::emerald_block;
Tile* Tile::emerald_ore;
Tile* Tile::enchanting_table;
Tile* Tile::end_brick_stairs;
Tile* Tile::end_bricks;
Tile* Tile::end_gateway;
Tile* Tile::end_portal;
Tile* Tile::end_portal_frame;
Tile* Tile::end_rod;
Tile* Tile::end_stone;
Tile* Tile::ender_chest;
Tile* Tile::farmland;
Tile* Tile::fence;
Tile* Tile::fence_gate;
Tile* Tile::fire;
Tile* Tile::fletching_table;
Tile* Tile::flower_pot;
Tile* Tile::flowing_lava;
Tile* Tile::flowing_water;
Tile* Tile::format_version;
Tile* Tile::frame;
Tile* Tile::frosted_ice;
Tile* Tile::furnace;
Tile* Tile::glass;
Tile* Tile::glass_pane;
Tile* Tile::glowingobsidian;
Tile* Tile::glowstone;
Tile* Tile::gold_block;
Tile* Tile::gold_ore;
Tile* Tile::golden_rail;
Tile* Tile::granite_stairs;
Tile* Tile::grass;
Tile* Tile::grass_path;
Tile* Tile::gravel;
Tile* Tile::gray_glazed_terracotta;
Tile* Tile::green_glazed_terracotta;
Tile* Tile::grindstone;
Tile* Tile::hardened_clay;
Tile* Tile::hay_block;
Tile* Tile::heavy_weighted_pressure_plate;
Tile* Tile::hopper;
Tile* Tile::ice;
Tile* Tile::info_update;
Tile* Tile::info_update2;
Tile* Tile::invisibleBedrock;
Tile* Tile::iron_bars;
Tile* Tile::iron_block;
Tile* Tile::iron_door;
Tile* Tile::iron_ore;
Tile* Tile::iron_trapdoor;
Tile* Tile::jigsaw;
Tile* Tile::jukebox;
Tile* Tile::jungle_button;
Tile* Tile::jungle_door;
Tile* Tile::jungle_fence_gate;
Tile* Tile::jungle_pressure_plate;
Tile* Tile::jungle_stairs;
Tile* Tile::jungle_standing_sign;
Tile* Tile::jungle_trapdoor;
Tile* Tile::jungle_wall_sign;
Tile* Tile::kelp;
Tile* Tile::ladder;
Tile* Tile::lantern;
Tile* Tile::lapis_block;
Tile* Tile::lapis_ore;
Tile* Tile::lava;
Tile* Tile::lava_cauldron;
Tile* Tile::leaves;
Tile* Tile::leaves2;
Tile* Tile::lectern;
Tile* Tile::lever;
Tile* Tile::light_block;
Tile* Tile::light_blue_glazed_terracotta;
Tile* Tile::light_weighted_pressure_plate;
Tile* Tile::lime_glazed_terracotta;
Tile* Tile::lit_blast_furnace;
Tile* Tile::lit_furnace;
Tile* Tile::lit_pumpkin;
Tile* Tile::lit_redstone_lamp;
Tile* Tile::lit_redstone_ore;
Tile* Tile::lit_smoker;
Tile* Tile::log;
Tile* Tile::log2;
Tile* Tile::loom;
Tile* Tile::magenta_glazed_terracotta;
Tile* Tile::magma;
Tile* Tile::melon_block;
Tile* Tile::melon_stem;
Tile* Tile::mob_spawner;
Tile* Tile::monster_egg;
Tile* Tile::mossy_cobblestone;
Tile* Tile::mossy_cobblestone_stairs;
Tile* Tile::mossy_stone_brick_stairs;
Tile* Tile::movingBlock;
Tile* Tile::mycelium;
Tile* Tile::nether_brick;
Tile* Tile::nether_brick_fence;
Tile* Tile::nether_brick_stairs;
Tile* Tile::nether_wart;
Tile* Tile::nether_wart_block;
Tile* Tile::netherrack;
Tile* Tile::netherreactor;
Tile* Tile::normal_stone_stairs;
Tile* Tile::noteblock;
Tile* Tile::oak_stairs;
Tile* Tile::observer;
Tile* Tile::obsidian;
Tile* Tile::orange_glazed_terracotta;
Tile* Tile::packed_ice;
Tile* Tile::pink_glazed_terracotta;
Tile* Tile::piston;
Tile* Tile::pistonArmCollision;
Tile* Tile::planks;
Tile* Tile::podzol;
Tile* Tile::polished_andesite_stairs;
Tile* Tile::polished_diorite_stairs;
Tile* Tile::polished_granite_stairs;
Tile* Tile::portal;
Tile* Tile::potatoes;
Tile* Tile::powered_comparator;
Tile* Tile::powered_repeater;
Tile* Tile::prismarine;
Tile* Tile::prismarine_bricks_stairs;
Tile* Tile::prismarine_stairs;
Tile* Tile::pumpkin;
Tile* Tile::pumpkin_stem;
Tile* Tile::purple_glazed_terracotta;
Tile* Tile::purpur_block;
Tile* Tile::purpur_stairs;
Tile* Tile::quartz_block;
Tile* Tile::quartz_ore;
Tile* Tile::quartz_stairs;
Tile* Tile::rail;
Tile* Tile::red_flower;
Tile* Tile::red_glazed_terracotta;
Tile* Tile::red_mushroom;
Tile* Tile::red_mushroom_block;
Tile* Tile::red_nether_brick;
Tile* Tile::red_nether_brick_stairs;
Tile* Tile::red_sandstone;
Tile* Tile::red_sandstone_stairs;
Tile* Tile::redstone_block;
Tile* Tile::redstone_lamp;
Tile* Tile::redstone_ore;
Tile* Tile::redstone_torch;
Tile* Tile::redstone_wire;
Tile* Tile::reeds;
Tile* Tile::repeating_command_block;
Tile* Tile::reserved6;
Tile* Tile::sand;
Tile* Tile::sandstone;
Tile* Tile::sandstone_stairs;
Tile* Tile::sapling;
Tile* Tile::scaffolding;
Tile* Tile::seaLantern;
Tile* Tile::sea_pickle;
Tile* Tile::seagrass;
Tile* Tile::shulker_box;
Tile* Tile::silver_glazed_terracotta;
Tile* Tile::skull;
Tile* Tile::slime;
Tile* Tile::smithing_table;
Tile* Tile::smoker;
Tile* Tile::smooth_quartz_stairs;
Tile* Tile::smooth_red_sandstone_stairs;
Tile* Tile::smooth_sandstone_stairs;
Tile* Tile::smooth_stone;
Tile* Tile::snow;
Tile* Tile::snow_layer;
Tile* Tile::soul_sand;
Tile* Tile::sponge;
Tile* Tile::spruce_button;
Tile* Tile::spruce_door;
Tile* Tile::spruce_fence_gate;
Tile* Tile::spruce_pressure_plate;
Tile* Tile::spruce_stairs;
Tile* Tile::spruce_standing_sign;
Tile* Tile::spruce_trapdoor;
Tile* Tile::spruce_wall_sign;
Tile* Tile::stained_glass;
Tile* Tile::stained_glass_pane;
Tile* Tile::stained_hardened_clay;
Tile* Tile::standing_banner;
Tile* Tile::standing_sign;
Tile* Tile::stickyPistonArmCollision;
Tile* Tile::sticky_piston;
Tile* Tile::stone;
Tile* Tile::stone_brick_stairs;
Tile* Tile::stone_button;
Tile* Tile::stone_pressure_plate;
Tile* Tile::stone_slab;
Tile* Tile::stone_slab2;
Tile* Tile::stone_slab3;
Tile* Tile::stone_slab4;
Tile* Tile::stone_stairs;
Tile* Tile::stonebrick;
Tile* Tile::stonecutter;
Tile* Tile::stonecutter_block;
Tile* Tile::stripped_acacia_log;
Tile* Tile::stripped_birch_log;
Tile* Tile::stripped_dark_oak_log;
Tile* Tile::stripped_jungle_log;
Tile* Tile::stripped_oak_log;
Tile* Tile::stripped_spruce_log;
Tile* Tile::structure_block;
Tile* Tile::structure_void;
Tile* Tile::sweet_berry_bush;
Tile* Tile::tallgrass;
Tile* Tile::tnt;
Tile* Tile::torch;
Tile* Tile::trapdoor;
Tile* Tile::trapped_chest;
Tile* Tile::tripWire;
Tile* Tile::tripwire_hook;
Tile* Tile::turtle_egg;
Tile* Tile::undyed_shulker_box;
Tile* Tile::unlit_redstone_torch;
Tile* Tile::unpowered_comparator;
Tile* Tile::unpowered_repeater;
Tile* Tile::vine;
Tile* Tile::wall_banner;
Tile* Tile::wall_sign;
Tile* Tile::water;
Tile* Tile::waterlily;
Tile* Tile::web;
Tile* Tile::wheat;
Tile* Tile::white_glazed_terracotta;
Tile* Tile::wither_rose;
Tile* Tile::wood;
Tile* Tile::wooden_button;
Tile* Tile::wooden_door;
Tile* Tile::wooden_pressure_plate;
Tile* Tile::wooden_slab;
Tile* Tile::wool;
Tile* Tile::yellow_flower;
Tile* Tile::yellow_glazed_terracotta;

void Tile::initTiles(TextureManager* textureManager) {
	acacia_button = new AcaciaButtonTile();
	acacia_door = new AcaciaDoorTile();
	acacia_fence_gate = new AcaciaFenceGateTile();
	acacia_pressure_plate = new AcaciaPressurePlateTile();
	acacia_stairs = new AcaciaStairsTile();
	acacia_standing_sign = new AcaciaStandingSignTile();
	acacia_trapdoor = new AcaciaTrapdoorTile();
	acacia_wall_sign = new AcaciaWallSignTile();
	activator_rail = new ActivatorRailTile();
	air = new AirTile();
	allow = new AllowTile();
	andesite_stairs = new AndesiteStairsTile();
	anvil = new AnvilTile();
	bamboo = new BambooTile();
	bamboo_sapling = new BambooSaplingTile();
	barrel = new BarrelTile();
	barrier = new BarrierTile();
	beacon = new BeaconTile();
	bed = new BedTile();
	bedrock = new BedrockTile();
	beetroot = new BeetrootTile();
	bell = new BellTile();
	birch_button = new BirchButtonTile();
	birch_door = new BirchDoorTile();
	birch_fence_gate = new BirchFenceGateTile();
	birch_pressure_plate = new BirchPressurePlateTile();
	birch_stairs = new BirchStairsTile();
	birch_standing_sign = new BirchStandingSignTile();
	birch_trapdoor = new BirchTrapdoorTile();
	birch_wall_sign = new BirchWallSignTile();
	black_glazed_terracotta = new BlackGlazedTerracottaTile();
	blast_furnace = new BlastFurnaceTile();
	blue_glazed_terracotta = new BlueGlazedTerracottaTile();
	blue_ice = new BlueIceTile();
	bone_block = new BoneBlockTile();
	bookshelf = new BookshelfTile();
	border_block = new BorderBlockTile();
	brewing_stand = new BrewingStandTile();
	brick_block = new BrickBlockTile();
	brick_stairs = new BrickStairsTile();
	brown_glazed_terracotta = new BrownGlazedTerracottaTile();
	brown_mushroom = new BrownMushroomTile();
	brown_mushroom_block = new BrownMushroomBlockTile();
	bubble_column = new BubbleColumnTile();
	cactus = new CactusTile();
	cake = new CakeTile();
	camera = new CameraTile();
	campfire = new CampfireTile();
	carpet = new CarpetTile();
	carrots = new CarrotsTile();
	cartography_table = new CartographyTableTile();
	carved_pumpkin = new CarvedPumpkinTile();
	cauldron = new CauldronTile();
	chain_command_block = new ChainCommandBlockTile();
	chest = new ChestTile();
	chorus_flower = new ChorusFlowerTile();
	chorus_plant = new ChorusPlantTile();
	clay = new ClayTile();
	coal_block = new CoalBlockTile();
	coal_ore = new CoalOreTile();
	cobblestone = new CobblestoneTile();
	cobblestone_wall = new CobblestoneWallTile();
	cocoa = new CocoaTile();
	command_block = new CommandBlockTile();
	composter = new ComposterTile();
	concrete = new ConcreteTile();
	concretePowder = new ConcretepowderTile();
	conduit = new ConduitTile();
	coral = new CoralTile();
	coral_block = new CoralBlockTile();
	coral_fan = new CoralFanTile();
	coral_fan_dead = new CoralFanDeadTile();
	coral_fan_hang = new CoralFanHangTile();
	coral_fan_hang2 = new CoralFanHang2Tile();
	coral_fan_hang3 = new CoralFanHang3Tile();
	crafting_table = new CraftingTableTile();
	cyan_glazed_terracotta = new CyanGlazedTerracottaTile();
	dark_oak_button = new DarkOakButtonTile();
	dark_oak_door = new DarkOakDoorTile();
	dark_oak_fence_gate = new DarkOakFenceGateTile();
	dark_oak_pressure_plate = new DarkOakPressurePlateTile();
	dark_oak_stairs = new DarkOakStairsTile();
	dark_oak_trapdoor = new DarkOakTrapdoorTile();
	dark_prismarine_stairs = new DarkPrismarineStairsTile();
	darkoak_standing_sign = new DarkoakStandingSignTile();
	darkoak_wall_sign = new DarkoakWallSignTile();
	daylight_detector = new DaylightDetectorTile();
	daylight_detector_inverted = new DaylightDetectorInvertedTile();
	deadbush = new DeadbushTile();
	deny = new DenyTile();
	detector_rail = new DetectorRailTile();
	diamond_block = new DiamondBlockTile();
	diamond_ore = new DiamondOreTile();
	diorite_stairs = new DioriteStairsTile();
	dirt = new DirtTile();
	dispenser = new DispenserTile();
	double_plant = new DoublePlantTile();
	double_stone_slab = new DoubleStoneSlabTile();
	double_stone_slab2 = new DoubleStoneSlab2Tile();
	double_stone_slab3 = new DoubleStoneSlab3Tile();
	double_stone_slab4 = new DoubleStoneSlab4Tile();
	double_wooden_slab = new DoubleWoodenSlabTile();
	dragon_egg = new DragonEggTile();
	dried_kelp_block = new DriedKelpBlockTile();
	dropper = new DropperTile();
	emerald_block = new EmeraldBlockTile();
	emerald_ore = new EmeraldOreTile();
	enchanting_table = new EnchantingTableTile();
	end_brick_stairs = new EndBrickStairsTile();
	end_bricks = new EndBricksTile();
	end_gateway = new EndGatewayTile();
	end_portal = new EndPortalTile();
	end_portal_frame = new EndPortalFrameTile();
	end_rod = new EndRodTile();
	end_stone = new EndStoneTile();
	ender_chest = new EnderChestTile();
	farmland = new FarmlandTile();
	fence = new FenceTile();
	fence_gate = new FenceGateTile();
	fire = new FireTile();
	fletching_table = new FletchingTableTile();
	flower_pot = new FlowerPotTile();
	flowing_lava = new FlowingLavaTile();
	flowing_water = new FlowingWaterTile();
	format_version = new FormatVersionTile();
	frame = new FrameTile();
	frosted_ice = new FrostedIceTile();
	furnace = new FurnaceTile();
	glass = new GlassTile();
	glass_pane = new GlassPaneTile();
	glowingobsidian = new GlowingobsidianTile();
	glowstone = new GlowstoneTile();
	gold_block = new GoldBlockTile();
	gold_ore = new GoldOreTile();
	golden_rail = new GoldenRailTile();
	granite_stairs = new GraniteStairsTile();
	grass = new GrassTile();
	grass_path = new GrassPathTile();
	gravel = new GravelTile();
	gray_glazed_terracotta = new GrayGlazedTerracottaTile();
	green_glazed_terracotta = new GreenGlazedTerracottaTile();
	grindstone = new GrindstoneTile();
	hardened_clay = new HardenedClayTile();
	hay_block = new HayBlockTile();
	heavy_weighted_pressure_plate = new HeavyWeightedPressurePlateTile();
	hopper = new HopperTile();
	ice = new IceTile();
	info_update = new InfoUpdateTile();
	info_update2 = new InfoUpdate2Tile();
	invisibleBedrock = new InvisiblebedrockTile();
	iron_bars = new IronBarsTile();
	iron_block = new IronBlockTile();
	iron_door = new IronDoorTile();
	iron_ore = new IronOreTile();
	iron_trapdoor = new IronTrapdoorTile();
	jigsaw = new JigsawTile();
	jukebox = new JukeboxTile();
	jungle_button = new JungleButtonTile();
	jungle_door = new JungleDoorTile();
	jungle_fence_gate = new JungleFenceGateTile();
	jungle_pressure_plate = new JunglePressurePlateTile();
	jungle_stairs = new JungleStairsTile();
	jungle_standing_sign = new JungleStandingSignTile();
	jungle_trapdoor = new JungleTrapdoorTile();
	jungle_wall_sign = new JungleWallSignTile();
	kelp = new KelpTile();
	ladder = new LadderTile();
	lantern = new LanternTile();
	lapis_block = new LapisBlockTile();
	lapis_ore = new LapisOreTile();
	lava = new LavaTile();
	lava_cauldron = new LavaCauldronTile();
	leaves = new LeavesTile();
	leaves2 = new Leaves2Tile();
	lectern = new LecternTile();
	lever = new LeverTile();
	light_block = new LightBlockTile();
	light_blue_glazed_terracotta = new LightBlueGlazedTerracottaTile();
	light_weighted_pressure_plate = new LightWeightedPressurePlateTile();
	lime_glazed_terracotta = new LimeGlazedTerracottaTile();
	lit_blast_furnace = new LitBlastFurnaceTile();
	lit_furnace = new LitFurnaceTile();
	lit_pumpkin = new LitPumpkinTile();
	lit_redstone_lamp = new LitRedstoneLampTile();
	lit_redstone_ore = new LitRedstoneOreTile();
	lit_smoker = new LitSmokerTile();
	log = new LogTile();
	log2 = new Log2Tile();
	loom = new LoomTile();
	magenta_glazed_terracotta = new MagentaGlazedTerracottaTile();
	magma = new MagmaTile();
	melon_block = new MelonBlockTile();
	melon_stem = new MelonStemTile();
	mob_spawner = new MobSpawnerTile();
	monster_egg = new MonsterEggTile();
	mossy_cobblestone = new MossyCobblestoneTile();
	mossy_cobblestone_stairs = new MossyCobblestoneStairsTile();
	mossy_stone_brick_stairs = new MossyStoneBrickStairsTile();
	movingBlock = new MovingblockTile();
	mycelium = new MyceliumTile();
	nether_brick = new NetherBrickTile();
	nether_brick_fence = new NetherBrickFenceTile();
	nether_brick_stairs = new NetherBrickStairsTile();
	nether_wart = new NetherWartTile();
	nether_wart_block = new NetherWartBlockTile();
	netherrack = new NetherrackTile();
	netherreactor = new NetherreactorTile();
	normal_stone_stairs = new NormalStoneStairsTile();
	noteblock = new NoteblockTile();
	oak_stairs = new OakStairsTile();
	observer = new ObserverTile();
	obsidian = new ObsidianTile();
	orange_glazed_terracotta = new OrangeGlazedTerracottaTile();
	packed_ice = new PackedIceTile();
	pink_glazed_terracotta = new PinkGlazedTerracottaTile();
	piston = new PistonTile();
	pistonArmCollision = new PistonarmcollisionTile();
	planks = new PlanksTile();
	podzol = new PodzolTile();
	polished_andesite_stairs = new PolishedAndesiteStairsTile();
	polished_diorite_stairs = new PolishedDioriteStairsTile();
	polished_granite_stairs = new PolishedGraniteStairsTile();
	portal = new PortalTile();
	potatoes = new PotatoesTile();
	powered_comparator = new PoweredComparatorTile();
	powered_repeater = new PoweredRepeaterTile();
	prismarine = new PrismarineTile();
	prismarine_bricks_stairs = new PrismarineBricksStairsTile();
	prismarine_stairs = new PrismarineStairsTile();
	pumpkin = new PumpkinTile();
	pumpkin_stem = new PumpkinStemTile();
	purple_glazed_terracotta = new PurpleGlazedTerracottaTile();
	purpur_block = new PurpurBlockTile();
	purpur_stairs = new PurpurStairsTile();
	quartz_block = new QuartzBlockTile();
	quartz_ore = new QuartzOreTile();
	quartz_stairs = new QuartzStairsTile();
	rail = new RailTile();
	red_flower = new RedFlowerTile();
	red_glazed_terracotta = new RedGlazedTerracottaTile();
	red_mushroom = new RedMushroomTile();
	red_mushroom_block = new RedMushroomBlockTile();
	red_nether_brick = new RedNetherBrickTile();
	red_nether_brick_stairs = new RedNetherBrickStairsTile();
	red_sandstone = new RedSandstoneTile();
	red_sandstone_stairs = new RedSandstoneStairsTile();
	redstone_block = new RedstoneBlockTile();
	redstone_lamp = new RedstoneLampTile();
	redstone_ore = new RedstoneOreTile();
	redstone_torch = new RedstoneTorchTile();
	redstone_wire = new RedstoneWireTile();
	reeds = new ReedsTile();
	repeating_command_block = new RepeatingCommandBlockTile();
	reserved6 = new Reserved6Tile();
	sand = new SandTile();
	sandstone = new SandstoneTile();
	sandstone_stairs = new SandstoneStairsTile();
	sapling = new SaplingTile();
	scaffolding = new ScaffoldingTile();
	seaLantern = new SealanternTile();
	sea_pickle = new SeaPickleTile();
	seagrass = new SeagrassTile();
	shulker_box = new ShulkerBoxTile();
	silver_glazed_terracotta = new SilverGlazedTerracottaTile();
	skull = new SkullTile();
	slime = new SlimeTile();
	smithing_table = new SmithingTableTile();
	smoker = new SmokerTile();
	smooth_quartz_stairs = new SmoothQuartzStairsTile();
	smooth_red_sandstone_stairs = new SmoothRedSandstoneStairsTile();
	smooth_sandstone_stairs = new SmoothSandstoneStairsTile();
	smooth_stone = new SmoothStoneTile();
	snow = new SnowTile();
	snow_layer = new SnowLayerTile();
	soul_sand = new SoulSandTile();
	sponge = new SpongeTile();
	spruce_button = new SpruceButtonTile();
	spruce_door = new SpruceDoorTile();
	spruce_fence_gate = new SpruceFenceGateTile();
	spruce_pressure_plate = new SprucePressurePlateTile();
	spruce_stairs = new SpruceStairsTile();
	spruce_standing_sign = new SpruceStandingSignTile();
	spruce_trapdoor = new SpruceTrapdoorTile();
	spruce_wall_sign = new SpruceWallSignTile();
	stained_glass = new StainedGlassTile();
	stained_glass_pane = new StainedGlassPaneTile();
	stained_hardened_clay = new StainedHardenedClayTile();
	standing_banner = new StandingBannerTile();
	standing_sign = new StandingSignTile();
	stickyPistonArmCollision = new StickypistonarmcollisionTile();
	sticky_piston = new StickyPistonTile();
	stone = new StoneTile();
	stone_brick_stairs = new StoneBrickStairsTile();
	stone_button = new StoneButtonTile();
	stone_pressure_plate = new StonePressurePlateTile();
	stone_slab = new StoneSlabTile();
	stone_slab2 = new StoneSlab2Tile();
	stone_slab3 = new StoneSlab3Tile();
	stone_slab4 = new StoneSlab4Tile();
	stone_stairs = new StoneStairsTile();
	stonebrick = new StonebrickTile();
	stonecutter = new StonecutterTile();
	stonecutter_block = new StonecutterBlockTile();
	stripped_acacia_log = new StrippedAcaciaLogTile();
	stripped_birch_log = new StrippedBirchLogTile();
	stripped_dark_oak_log = new StrippedDarkOakLogTile();
	stripped_jungle_log = new StrippedJungleLogTile();
	stripped_oak_log = new StrippedOakLogTile();
	stripped_spruce_log = new StrippedSpruceLogTile();
	structure_block = new StructureBlockTile();
	structure_void = new StructureVoidTile();
	sweet_berry_bush = new SweetBerryBushTile();
	tallgrass = new TallgrassTile();
	tnt = new TntTile();
	torch = new TorchTile();
	trapdoor = new TrapdoorTile();
	trapped_chest = new TrappedChestTile();
	tripWire = new TripwireTile();
	tripwire_hook = new TripwireHookTile();
	turtle_egg = new TurtleEggTile();
	undyed_shulker_box = new UndyedShulkerBoxTile();
	unlit_redstone_torch = new UnlitRedstoneTorchTile();
	unpowered_comparator = new UnpoweredComparatorTile();
	unpowered_repeater = new UnpoweredRepeaterTile();
	vine = new VineTile();
	wall_banner = new WallBannerTile();
	wall_sign = new WallSignTile();
	water = new WaterTile();
	waterlily = new WaterlilyTile();
	web = new WebTile();
	wheat = new WheatTile();
	white_glazed_terracotta = new WhiteGlazedTerracottaTile();
	wither_rose = new WitherRoseTile();
	wood = new WoodTile();
	wooden_button = new WoodenButtonTile();
	wooden_door = new WoodenDoorTile();
	wooden_pressure_plate = new WoodenPressurePlateTile();
	wooden_slab = new WoodenSlabTile();
	wool = new WoolTile();
	yellow_flower = new YellowFlowerTile();
	yellow_glazed_terracotta = new YellowGlazedTerracottaTile();
}