const { merge } = require("webpack-merge");
const webpack = require("webpack");
const HtmlWebpackPlugin = require("html-webpack-plugin");
const commonConfig = require("./webpack.config.common");

module.exports = merge(commonConfig, {
	mode: "development",
	entry: [
		"react-hot-loader/patch", 
		"webpack-dev-server/client?http://localhost:8080", 
		"webpack/hot/only-dev-server",
		"./index.tsx",
	],
	devServer: {
		hot: true,
	},
	devtool: "cheap-module-source-map",
	plugins: [
		new webpack.HotModuleReplacementPlugin(),
		new HtmlWebpackPlugin({
			template: "index.ejs",
			templateParameters: {
				websocket: "192.168.178.41", // change to your esp32 ip address
			},
		}),
	],
});
