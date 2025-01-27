let unblurredStream = undefined;

// Blur local video. Only first local stream is supported for now.
function blur(enable = true) {
    if ( (enable && unblurredStream !== undefined) || (!enable && unblurredStream === undefined)) {
        //console.error("Blur: Can't unblur or blur a stream that's already blurred or unblurred.")
        return;
    }

    let videoTrack = document.getElementById("selfVideo").srcObject.getVideoTracks()[0];
    let videoStream = document.getElementById("selfVideo").srcObject;
    const canvas = new OffscreenCanvas(videoTrack.getSettings().width, videoTrack.getSettings().height);
    const ctx = canvas.getContext("2d");

    if (!enable){
        videoTrack.stop();
        document.getElementById("selfVideo").srcObject = unblurredStream;
        unblurredStream = undefined;
    }else{
        const selfieSegmentation = new SelfieSegmentation({
            locateFile: (file) =>
                `https://cdn.jsdelivr.net/npm/@mediapipe/selfie_segmentation/${file}`,
        });

        selfieSegmentation.setOptions({
            modelSelection: 0
        });

        selfieSegmentation.onResults(function(results){
            ctx.save();
            ctx.clearRect(
                0,
                0,
                canvas.width,
                canvas.height
            );
            ctx.drawImage(
                results.segmentationMask,
                0,
                0,
                canvas.width,
                canvas.height
            );

            ctx.globalCompositeOperation = "source-in";
            ctx.drawImage(
                results.image,
                0,
                0,
                canvas.width,
                canvas.height
            );

            // Only overwrite missing pixels.
            ctx.globalCompositeOperation = "destination-atop";
            ctx.filter = `blur(16px)`;
            ctx.drawImage(
                results.image,
                0,
                0,
                canvas.width,
                canvas.height
            );
            ctx.restore();
        });

        const trackProcessor = new MediaStreamTrackProcessor({ track: videoTrack });
        const trackGenerator = new MediaStreamTrackGenerator({ kind: 'video' });

        const transformer = new TransformStream({
            async transform(videoFrame, controller) {
                videoFrame.width = videoFrame.displayWidth;
                videoFrame.height = videoFrame.displayHeight;
                await selfieSegmentation.send({ image: videoFrame });

                const timestamp = videoFrame.timestamp;
                const newFrame = new VideoFrame(canvas, {timestamp});

                videoFrame.close();
                controller.enqueue(newFrame);
            }
        });

        trackProcessor.readable.pipeThrough(transformer).pipeTo(trackGenerator.writable);

        const processedStream = new MediaStream();
        unblurredStream = videoStream;
        processedStream.addTrack(trackGenerator);
        document.getElementById("selfVideo").srcObject = processedStream;

    }
}
