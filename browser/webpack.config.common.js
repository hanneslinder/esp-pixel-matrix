// shared config (dev and prod)
const { resolve } = require("path");
const MiniCssExtractPlugin = require("mini-css-extract-plugin");

module.exports = {
	resolve: {
		extensions: [".ts", ".tsx", ".js"],
	},
	context: resolve(__dirname, "./src"),
	module: {
		rules: [
			{
				test: /\.tsx?$/,
				loader: "ts-loader",
				include: /src/,
				exclude: "/node_modules/",
			},
			{
				test: /\.less$/,
				use: [MiniCssExtractPlugin.loader, "css-loader", "less-loader"],
			},
			{
				test: /\.svg$/,
				loader: "svg-inline-loader",
			},
		],
	},
	plugins: [
		new MiniCssExtractPlugin(),
	],
	performance: {
		hints: false,
	},
};
