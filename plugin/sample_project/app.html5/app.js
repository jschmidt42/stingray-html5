// TODO: Click on 3D browsers
// TODO: 2D HTML5 HUD
define(require => {
    'use strict';

    //noinspection NpmUsedModulesInstalled
    const _ = require('lodash');

    const stingray = window.stingray;
    const Application = stingray.Application;
    const Window = stingray.Window;
    const World = stingray.World;
    const Level = stingray.Level;
    const Unit = stingray.Unit;
    const Camera = stingray.Camera;
    const Mouse = stingray.Mouse;
    const Keyboard = stingray.Keyboard;
    const Vector3 = stingray.Vector3;
	const Vector2 = stingray.Vector2;
    const Quaternion = stingray.Quaternion;
    const Matrix4x4 = stingray.Matrix4x4;
    const WebView = stingray.WebView;
	const Gui     = stingray.Gui;

    /*
     * Store application resources and states.
     */
    let app = {
        closing: false,
        window: Window.get_main_window(),
        world: null,
        viewport: null,
        gui: null,
		overlay_gui: null,
		overlay_view: null,
		overlay_material: null,
		overlay_bitmap: null,
        camera: {
            name: 'core/appkit/units/camera/camera',
            unit: null,
            instance: null
        },
        level: {
            name: 'levels/sample'
        },
        shading_environment: null,
        input: {
            run: false,
            move: [0, 0, 0],
            pan: [0, 0, 0],
            click: null
        }
    };
	let web_page_url = "http://www.google.com"
	let web_view_material_resource_name = "html5_resources/web_view_2d"
    const Button = {
        Mouse: {
            left: Mouse.button_id("left"),
            right: Mouse.button_id("right"),

            pos: Mouse.axis_id('cursor')
        },
        Keyboard: {
            w: Keyboard.button_id('w'),
            a: Keyboard.button_id('a'),
            s: Keyboard.button_id('s'),
            d: Keyboard.button_id('d'),

            space: Keyboard.button_id("space"),
            left_ctrl: Keyboard.button_id("left ctrl"),
            left_shift: Keyboard.button_id("left shift"),

            esc: Keyboard.button_id("esc")
        }
    };

    /**
     * Setup various application settings.
     */
    function setupApplication () {

        // Print build information
        console.info(`Build: ${Application.build()}`);
        console.info(`Build identifier: ${Application.build_identifier()}`);
        console.info(`Platform: ${Application.platform()}`);
        console.info(`System information: ${Application.sysinfo()}`);

        // Print settings.ini
        let settings = Application.settings();
        console.log(`Settings \r\n${JSON.stringify(settings, null, 2)}`, settings);

        Window.set_show_cursor(null, true, true);
        Window.set_clip_cursor(null, false);
        Window.set_mouse_focus(null, true);

        let args = Application.argv();
        console.info(`Arguments: ${args.join(' ')}`);
    }

    /**
     * Create the main world.
     */
    function createWorld () {
        const viewportTemplateName = "default";

        app.world = Application.new_world();
        console.assert(app.world, 'Failed to create world');

        app.viewport = Application.create_viewport(app.world, viewportTemplateName);
        console.assert(app.viewport, 'Failed to create viewport');

        app.gui = World.create_screen_gui(app.world, null, null, false, true, true, true);
        console.assert(app.gui, 'Failed to create gui');
		app.overlay_gui = World.create_screen_gui(app.world, null, null, true, true, true, true);
		console.assert(app.overlay_gui, 'Failed to create overlay');
        console.info('World created');

		app.overlay_material = Gui.material(app.overlay_gui, web_view_material_resource_name)
		console.info('Overlay material created');
		app.overlay_view = WebView.create(web_page_url, app.window, app.overlay_material)
		console.info('Webview: ' + web_page_url);

		let res = Gui.resolution(app.viewport, app.window);
		app.overlay_bitmap = Gui.bitmap(app.overlay_gui, app.overlay_material,
										Vector2.create(0,0), 10,
										Vector2.create(res[0],res[1]))
    }

    /**
     * Create the main camera.
     */
    function createCamera () {
        app.camera.unit = World.spawn_unit(app.world, app.camera.name, null, Matrix4x4.identity());
        console.assert(app.camera.unit, "Failed to spawn camera unit");
        app.camera.instance = Unit.camera(app.camera.unit, 0);
        console.assert(app.camera.instance, "Failed to get camera object from unit");

        let position = [0, 0, 3];
        let orientation = [0, 0, 0, 1];

        Unit.set_local_position(app.camera.unit, 0, position);
        Camera.set_local_position(app.camera.instance, app.camera.unit, position);
        Unit.set_local_rotation(app.camera.unit, 0, orientation);
        Camera.set_local_rotation(app.camera.instance, app.camera.unit, orientation);

        console.info('Camera created');
    }

    /**
     * Load the sample level.
     * @param levelName
     */
    function loadLevel (levelName) {
        app.level = World.load_level(app.world, levelName, levelName, Matrix4x4.identity());
        console.assert(app.level, 'Failed to create level');
        app.level.name = levelName;

        Level.spawn_background(app.level);

        // Load the shading environment for the level if there is one.
        let env_name = null;
        if (Level.has_data(app.level, "shading_environment"))
            env_name = Level.get_data(app.level, "shading_environment");

        if (!env_name) {
            console.warn("Warning: No shading environment set in Level, applying default");
            let default_shading_environment_new = "core/stingray_renderer/environments/midday/midday";
            let default_shading_environment_old = "core/rendering/default_outdoor";
            env_name = default_shading_environment_new;
            if (Application.can_get("shading_environment", default_shading_environment_old) &&
                !Application.can_get("shading_environment", default_shading_environment_new)) {
                env_name = default_shading_environment_old;
            }
        }

        app.shading_environment = World.create_shading_environment(app.world, env_name);
        World.set_shading_environment(app.world, app.shading_environment, env_name);

        console.info(`Level ${levelName} loaded`);
    }

    /**
     * Initialize the web app application.
     */
    function init() {
        setupApplication();
        createWorld();
        createCamera();

        loadLevel(app.level.name);

        console.info('Web app loaded');
    }

    /**
     * Reset user inputs.
     */
    function resetInput () {
        app.input.run = false;
        app.input.pan = app.input.move = [0, 0 ,0];
    }

    /**
     * Read user inputs.
     */
    function readInput() {

        // Check mouse click
        if (Mouse.pressed(Button.Mouse.left)) {
            app.input.click = Mouse.axis(Button.Mouse.pos);
            console.log('click', app.input.click);
        } else {
            app.input.click = null;
        }

        // Only update camera if the right mouse button is pressed.
        if (Mouse.button(Button.Mouse.right) < 1)
            return resetInput();

        let input = app.input;
        let pan = Mouse.axis(Mouse.axis_id("mouse"));
        let move = [
            Keyboard.button(Button.Keyboard.d) - Keyboard.button(Button.Keyboard.a),
            Keyboard.button(Button.Keyboard.w) - Keyboard.button(Button.Keyboard.s),
            0
        ];

        input.run = Keyboard.button(Button.Keyboard.left_shift) > 0;
        input.move = Vector3.normalize(move);
        input.pan = pan;
    }

    /**
     * Update camera from captured user inputs.
     * @param {number} dt
     */
    function updateCamera(dt) {
        const mouseScale = 0.01;
        const yawSpeed = 1;
        const pitchSpeed = 1;
        const pitchMin = -90;
        const pitchMax = 90;
        const translationSpeed = Vector3.multiply([1, 1, 1], app.input.run ? 4.0 : 1.0);

        function computeRotation(input) {
            let q_original = Unit.local_rotation(app.camera.unit, 0);
            let m_original = Matrix4x4.from_quaternion(q_original);

            let yaw = 0;
            let pitch = 0;

            let mouseAxis = Mouse.axis(Mouse.axis_id("mouse"));
            if (!Vector3.equal(mouseAxis, Vector3.zero())) {
                yaw = input.pan[0] * mouseScale;
                pitch = input.pan[1] * mouseScale;
            }

            let q_yaw = Quaternion.axis_angle([0,0,1], -yaw * yawSpeed);
            let m_x = Matrix4x4.x(m_original);
            let q_pitch = Quaternion.axis_angle(m_x, -pitch * pitchSpeed);

            let q_frame = Quaternion.multiply(q_yaw, q_pitch);
            let q_new = Quaternion.multiply(q_frame, q_original);

            // Clamp yaw and pitch
            if (pitchMin && pitchMax) {
                let pitchAngle = Math.acos(Vector3.dot(Quaternion.up(q_new), [0,0,1]));
                let aboveHorizon = Quaternion.forward(q_new)[2] > 0;
                let adjustment = 0;

                if (aboveHorizon) {
                    let limit = pitchMax * Math.PI/180.0;
                    adjustment = Math.min(0, limit - pitchAngle);
                } else {
                    let limit = pitchMin * Math.PI/180.0;
                    adjustment = Math.max(0, limit + pitchAngle);
                }

                if (adjustment !== 0) {
                    let adjustment_rotation = Quaternion.axis_angle([1,0,0],adjustment);
                    q_new = Quaternion.multiply(q_new, adjustment_rotation);
                }
            }

            return q_new;
        }

        function computeTranslation(input, q, dt) {
            let inputMove = input.move;
            let pose = Matrix4x4.from_quaternion(q);

            let frameMove = Vector3.multiply(Vector3.multiply_elements(inputMove, translationSpeed), dt);
            let move = Matrix4x4.transform(pose, frameMove);

            let cameraPos = Unit.local_position(app.camera.unit, 0);
            return Vector3.add(cameraPos, move);
        }

        let q = computeRotation(app.input, dt);
        let p = computeTranslation(app.input, q, dt);

        Unit.set_local_rotation(app.camera.unit, 0, q);
        Unit.set_local_position(app.camera.unit, 0, p);
    }

    /**
     * Main update loop.
     * @param dt
     */
    function update(dt) {
        if (app.closing)
            return;

        // Read user input
        readInput();

        // Check if the user requested to quit the application.
        if (Keyboard.pressed(Button.Keyboard.esc)) {
            return quit();
        }

        // Update the camera settings based on the read user inputs.
        updateCamera(dt);

        // User clicked the engine window.
        if (app.input.click) {
            // Lets try to see which browser was picked.
            let windowRect = Window.rect(app.window);
            let hitPos1 = Camera.screen_to_world(app.camera.instance, app.input.click, 0, windowRect[2], windowRect[3]);
            let hitPos2 = Camera.screen_to_world(app.camera.instance, app.input.click, 1, windowRect[2], windowRect[3]);
            let ray = Vector3.normalize(Vector3.subtract(hitPos2, hitPos1));
            WebView.pick(app.input.click, hitPos1, ray);
        }

        // Update the level and world.
        Level.trigger_level_update(app.level);
        World.update(app.world, dt);
    }

    /**
     * Render scene elements and worlds.
     */
    function render() {
        if (app.closing)
            return;
		let res = Gui.resolution(app.viewport, app.window);
		WebView.render(app.overlay_view);
		Gui.update_bitmap(app.overlay_gui, app.overlay_bitmap, app.overlay_material,
						  Vector2.create(0,0), 10, Vector2.create(res[0],res[1]))
        Application.render_world(app.world, app.camera.instance, app.viewport, app.shading_environment, app.window);
    }

    /**
     * Shutdown web app.
     */
    function shutdown() {
		Gui.destroy_bitmap(app.overlay_gui, app.overlay_bitmap);
		WebView.destroy(app.overlay_view);
        Application.release_world(app.world);
        return quit();
    }

    /**
     * Request to quit web app.
     * @param exitCode
     */
    function quit (exitCode = 0) {
        app.closing = true;
        return Application.quit(exitCode);
    }

    return _.merge(app, {
        init,
        update,
        render,
        shutdown
    });
});
