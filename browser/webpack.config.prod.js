const { merge } = require("webpack-merge");
const HtmlWebpackPlugin = require("html-webpack-plugin");
const CssMinimizerPlugin = require("css-minimizer-webpack-plugin");
const { resolve } = require("path");
const exec = require('child_process').exec;

const commonConfig = require("./webpack.config.common");

module.exports = merge(commonConfig, {
	mode: "production",
	entry: "./index.tsx",
	output: {
		filename: "main.js",
		path: resolve(__dirname, "./dist"),
		publicPath: "/",
	},
	devtool: "source-map",
	optimization: {
    minimizer: [
			`...`,
      new CssMinimizerPlugin(),
    ],
  },
	plugins: [
		new HtmlWebpackPlugin({
			template: "index.ejs",
			templateParameters: {
				websocket: "%websocketUrl%", // gets replaced by esp32 webserver with ip address
			},
		}),
		{
      apply: (compiler) => {
        compiler.hooks.afterEmit.tap('AfterEmitPlugin', (compilation) => {
          exec('node copyArtifact.js', (err, stdout, stderr) => {
            if (stdout) process.stdout.write(stdout);
            if (stderr) process.stderr.write(stderr);
          });
        });
      }
    }
	],
});
