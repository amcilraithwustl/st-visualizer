const ForkTsCheckerWebpackPlugin = require("fork-ts-checker-webpack-plugin");
const HtmlWebpackPlugin = require("html-webpack-plugin");

module.exports = [
  // new ForkTsCheckerWebpackPlugin(), //This determines whether we do a ts check beforehand
  new HtmlWebpackPlugin({
    meta: {
      "Content-Security-Policy": {
        "http-equiv": "Content-Security-Policy",
        content: "script-src 'self' 'unsafe-inline'; object-src 'self'",
      },
    },
  }),
];
