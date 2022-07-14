import * as React from 'react';
import * as ReactDOM from 'react-dom';
import {App} from "../imports/App";

function render() {
    ReactDOM.render(<App/>, document.getElementById('react-target'));
}

render();
