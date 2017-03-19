(function () {
    'use strict';

    // Configure common RequireJS modules.
    let thridPartyRootHref = '//3rdparty';
    if (window.location.protocol === 'file:') {
        thridPartyRootHref = window.location.pathname.slice(1);
        thridPartyRootHref = thridPartyRootHref.substr(0, thridPartyRootHref.lastIndexOf('/'));
        thridPartyRootHref = thridPartyRootHref.substr(0, thridPartyRootHref.lastIndexOf('/'));
        thridPartyRootHref += "/3rdparty.html5"
    }

    require.config({
        paths: {
            // 3rdparty
            'lodash': thridPartyRootHref + '/lodash.min',
            'jquery': thridPartyRootHref + '/jquery.min'
        }
    });

    // Load main app
    require(['./app'], app => {
        app.init();
        window.render = app.render;
        window.update = app.update;
        window.shutdown = app.shutdown;
    });
})();
