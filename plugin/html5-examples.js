define(require => {
    "use strict";

    const engineService = require('services/engine-service');

    function showHud () {
        return engineService.evaluateScript(`(require "html5_examples/html5_hud")()`)
            .then(result => console.info(JSON.stringify(result)))
            .catch(err => console.error('Failed to execute HTML5 example:\r\n' + err)); 
    }

    return {
        showHud
    };
});