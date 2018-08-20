import bpy
import os

# don't show splash
bpy.context.user_preferences.view.show_splash = False

# remove objects
for bo in bpy.data.objects:
	bpy.data.objects.remove(bo)

obj = os.getenv("OPEN_OBJ")
bpy.ops.import_scene.obj(filepath = obj)
