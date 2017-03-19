/**
 * Stingray API
 * @namespace stingray
 */

/**
 * Engine application API
 * @namespace Application
 * @memberOf stingray
 * @property {function} build_identifier
 * @property {function} sysinfo
 * @property {function} new_world
 * @property {function} create_viewport
 * @property {function} can_get
 * @property {function} render_world
 * @property {function} release_world
 */

/**
 * Window API
 * @namespace Window
 * @memberOf stingray
 * @property {function} get_main_window
 * @property {function} set_mouse_focus
 * @property {function} set_show_cursor
 * @property {function} set_clip_cursor
 */

/**
 * World API
 * @namespace World
 * @memberOf stingray
 * @property {function} create_screen_gui
 * @property {function} spawn_unit
 * @property {function} load_level
 * @property {function} create_shading_environment
 * @property {function} set_shading_environment
 */

/**
 * Level API
 * @namespace Level
 * @memberOf stingray
 * @property {function} spawn_background
 * @property {function} has_data
 * @property {function} get_data
 * @property {function} trigger_level_update
 */


/**
 * Unit API
 * @namespace Unit
 * @memberOf stingray
 * @property {function} set_local_position
 * @property {function} set_local_rotation
 * @property {function} local_rotation
 * @property {function} local_position
 */

/**
 * Camera API
 * @namespace Camera
 * @memberOf stingray
 * @property {function} set_local_position
 * @property {function} set_local_rotation
 */

/**
 * @typedef {object} InputController
 * @property {function} axis_id
 * @property {function} button_id
 * @property {function} pressed
 */

/**
 * Mouse API
 * @namespace Mouse
 * @memberOf stingray
 */

/**
 * Keyboard API
 * @namespace Keyboard
 * @memberOf stingray
 */

/**
 * Vector3 API
 * @namespace Vector3
 * @memberOf stingray
 * @property {function} multiply_elements
 * @property {function} axis_angle
 * @property {function} axis_angle
 * @property {function} axis_angle
 *
 */

/**
 * Quaternion API
 * @namespace Quaternion
 * @memberOf stingray
 * @property {function} axis_angle
 */

/**
 * Matrix4x4 API
 * @namespace Matrix4x4
 * @memberOf stingray
 * @property {function} from_quaternion
 */
