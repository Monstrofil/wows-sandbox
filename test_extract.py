import ConstantsUtils
import CommonConsumables.ConsumableConstants as CC

print "=== Ship Types ==="
try:
    import m49789fe5 as Defaults
    if hasattr(Defaults, "ShipTypes"):
        st = Defaults.ShipTypes
        for name in sorted(dir(st)):
            if not name.startswith("_"):
                print "  %s = %s" % (name, getattr(st, name))
except Exception as e:
    print "  Error:", e

print ""
print "=== Consumable Types (first 15) ==="
count = 0
for name in sorted(dir(CC.ConsumablesTypes)):
    if name.startswith("CONSUMABLE_"):
        print "  %s = %s" % (name, getattr(CC.ConsumablesTypes, name))
        count += 1
        if count >= 15: break

print ""
print "=== Nations ==="
try:
    import m49789fe5 as Defaults
    if hasattr(Defaults, "Nations"):
        nations = Defaults.Nations
        for name in sorted(dir(nations)):
            if not name.startswith("_"):
                print "  %s = %s" % (name, getattr(nations, name))
except Exception as e:
    print "  Error:", e

print ""
print "Done."
